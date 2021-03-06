// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include <SOIL.h>
//#include <learnopengl/filesystem.h>

//SET USE_CANCEL_OUT_testpingpong2_shader to 1 to see the cancel out of the blurring effect whilst ping-ponging between fbos
#define USE_CANCEL_OUT_testpingpong2_shader 0

// Properties
const GLuint SCR_WIDTH = 800, SCR_HEIGHT = 600;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();
GLuint loadTexture(GLchar const * path);
void RenderScene(Shader &shader);
void RenderCube();
void RenderQuad();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));

// Delta
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Options
GLboolean bloom = true; // Change with 'Space'
GLfloat exposure = 1.0f; // Change with Q and E



/// @brief our texture id used by the FBO
//----------------------------------------------------------------------------------------------------------------------
GLuint m_textureID;
//----------------------------------------------------------------------------------------------------------------------
/// @brief our FBO id used by the FBO
//----------------------------------------------------------------------------------------------------------------------
GLuint m_fboID;
void createFramebufferObject()
{

  // Try to use a texture depth component
  glGenTextures(1, &m_textureID);
  glBindTexture(GL_TEXTURE_2D, m_textureID);
  //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  //glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

  glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

  glBindTexture(GL_TEXTURE_2D, 0);

  // create our FBO
  glGenFramebuffers(1, &m_fboID);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
  // disable the colour and read buffers as we only want depth
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  // attach our texture to the FBO

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D, m_textureID, 0);

  // switch back to window-system-provided framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static float m_lightAngle;

// The MAIN function, from here we start our application and run our Game loop
int main()
{





	// Init GLFW
	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Options
//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);

	// Setup and compile our shaders

    Shader shader("bloom.vs", "bloom.frag");
    Shader shaderLight("bloom.vs", "light_box.frag");
    Shader shaderBlur("blur.vs", "blur.frag");
    Shader testpingpong2("testpingpong2.vs", "testpingpong2.fs");
    Shader shaderShadow("ShadowVert.vs", "ShadowFrag.frag");
    // now create our FBO and texture
    createFramebufferObject();
    // we need to enable depth testing
    glEnable(GL_DEPTH_TEST);
    // set the depth comparison mode
    glDepthFunc(GL_LEQUAL);
    // set the bg to black
    glClearColor(0,0,0,1.0f);
    // enable face culling this will be switch to front and back when
    // rendering shadow or scene
    glEnable(GL_CULL_FACE);
    glPolygonOffset(1.1f,4);


    Shader shaderBloomFinal("bloom_final.vs", "bloom_final.frag");

	// Set samplers
	shaderBloomFinal.Use();
	glUniform1i(glGetUniformLocation(shaderBloomFinal.Program, "scene"), 0);
	glUniform1i(glGetUniformLocation(shaderBloomFinal.Program, "bloomBlur"), 1);

	// Light sources
	// - Positions
	std::vector<glm::vec3> lightPositions;
	lightPositions.push_back(glm::vec3(0.0f, 0.5f, 1.5f)); // back light
	lightPositions.push_back(glm::vec3(-4.0f, 0.5f, -3.0f));
	lightPositions.push_back(glm::vec3(3.0f, 0.5f, 1.0f));
	lightPositions.push_back(glm::vec3(-.8f, 2.4f, -1.0f));
	// - Colors
	std::vector<glm::vec3> lightColors;
	lightColors.push_back(glm::vec3(5.0f, 5.0f, 5.0f));
	lightColors.push_back(glm::vec3(5.5f, 0.0f, 0.0f));
	lightColors.push_back(glm::vec3(0.0f, 0.0f, 15.0f));
	lightColors.push_back(glm::vec3(0.0f, 1.5f, 0.0f));

	// Load textures
    GLuint woodTexture      = loadTexture("./resources/textures/wood.png");
    GLuint containerTexture = loadTexture("./resources/textures/container2.png");

	// Set up floating point framebuffer to render scene to
	GLuint hdrFBO;
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// - Create 2 floating point color buffers (1 for normal rendering, other for brightness treshold values)
	GLuint colorBuffers[2];
	glGenTextures(2, colorBuffers);
	for (GLuint i = 0; i < 2; i++) 
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // We clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}
	// - Create and attach depth buffer (renderbuffer)
	GLuint rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	// - Finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Ping pong framebuffer for blurring
	GLuint pingpongFBO[2];
	GLuint pingpongColorbuffers[2];
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);
	for (GLuint i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // We clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
		// Also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check and call events
        glfwPollEvents();
        Do_Movement();


        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //----------------------------------------------------------------------------------------------------------------------
        // Pass 1 render our Depth texture to the FBO
        //----------------------------------------------------------------------------------------------------------------------
        // enable culling
        glEnable(GL_CULL_FACE);

        // bind the FBO and render offscreen to the texture
        glBindFramebuffer(GL_FRAMEBUFFER,m_fboID);
        // bind the texture object to 0 (off ) - to the default texture
        glBindTexture(GL_TEXTURE_2D,0);
        // we need to render to the same size as the texture to avoid
        // distortions
        glViewport(0,0,SCR_WIDTH,SCR_HEIGHT);

        // Clear previous frame values
        glClear( GL_DEPTH_BUFFER_BIT);
        // as we are only rendering depth turn off the colour / alpha
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        // render only the back faces so we don't get too much self shadowing
        glCullFace(GL_FRONT);
        // draw the scene from the POV of the light using the function we need
//        drawScene(std::bind(&NGLScene::loadToLightPOVShader,this));
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //Draw Scene

        // 1. Render scene into floating point framebuffer
//		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 model;
            shader.Use();
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, woodTexture);
            // - set lighting uniforms
            for (GLuint i = 0; i < lightPositions.size(); i++)
            {
                glUniform3fv(glGetUniformLocation(shader.Program, ("lights[" + std::to_string(i) + "].Position").c_str()), 1, &lightPositions[i][0]);
                glUniform3fv(glGetUniformLocation(shader.Program, ("lights[" + std::to_string(i) + "].Color").c_str()), 1, &lightColors[i][0]);
            }

            glUniform3fv(glGetUniformLocation(shader.Program, "viewPos"), 1, &camera.Position[0]);
            // - create one large cube that acts as the floor
            model = glm::mat4();
            model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0));
            model = glm::scale(model, glm::vec3(25.0f, 1.0f, 25.0f));
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            RenderCube();
            // - then create multiple cubes as the scenery
            glBindTexture(GL_TEXTURE_2D, containerTexture);
            model = glm::mat4();
            model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            RenderCube();
            model = glm::mat4();
            model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            RenderCube();
            model = glm::mat4();
            model = glm::translate(model, glm::vec3(-1.0f, -1.0f, 2.0));
            model = glm::rotate(model, 60.0f, glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
            model = glm::scale(model, glm::vec3(2.0));
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            RenderCube();
            model = glm::mat4();
            model = glm::translate(model, glm::vec3(0.0f, 2.7f, 4.0));
            model = glm::rotate(model, 23.0f, glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
            model = glm::scale(model, glm::vec3(2.5));
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            RenderCube();
            model = glm::mat4();
            model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -3.0));
            model = glm::rotate(model, 124.0f, glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
            model = glm::scale(model, glm::vec3(2.0));
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            RenderCube();
            RenderCube();
            model = glm::mat4();
            model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0));
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            RenderCube();
            // - finally show all the light sources as bright cubes
            shaderLight.Use();
            glUniformMatrix4fv(glGetUniformLocation(shaderLight.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(glGetUniformLocation(shaderLight.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

            for (GLuint i = 0; i < lightPositions.size(); i++)
            {
                model = glm::mat4();
                model = glm::translate(model, glm::vec3(lightPositions[i]));
                model = glm::scale(model, glm::vec3(0.5f));
                glUniformMatrix4fv(glGetUniformLocation(shaderLight.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glUniform3fv(glGetUniformLocation(shaderLight.Program, "lightColor"), 1, &lightColors[i][0]);
                RenderCube();
            }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);



        //Now 2nd pass
        //----------------------------------------------------------------------------------------------------------------------
        // Pass two use the texture
        // now we have created the texture for shadows render the scene properly
        //----------------------------------------------------------------------------------------------------------------------
        // go back to our normal framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        // set the viewport to the screen dimensions
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        // enable colour rendering again (as we turned it off earlier)
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        // clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind the shadow texture
        glBindTexture(GL_TEXTURE_2D,m_textureID);

        // we need to generate the mip maps each time we bind
      //  glGenerateMipmap(GL_TEXTURE_2D);

        // now only cull back faces
        glDisable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        // render our scene with the shadow shader
//        drawScene(std::bind(&NGLScene::loadMatricesToShadowShader,this));
        //----------------------------------------------------------------------------------------------------------------------
        // this draws the debug texture on the quad
        //----------------------------------------------------------------------------------------------------------------------


        //Draw Scene
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        projection = glm::perspective(camera.Zoom, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        glm::mat4 normalMatrix=view*model;
        normalMatrix=glm::inverse(normalMatrix);

        shaderShadow.Use();
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "MV"), 1, GL_FALSE, glm::value_ptr(view*model));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "MVP"), 1, GL_FALSE, glm::value_ptr(projection*view*model));
        glUniformMatrix3fv(glGetUniformLocation(shaderShadow.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);

        // - set lighting uniforms




      // change the light angle
      m_lightAngle+=0.05;

      // now set this value and load to the shader


      Camera tmpLightcamera(glm::vec3((8.0*cos(m_lightAngle), 4.0 ,8.0*sin(m_lightAngle))));
      view = tmpLightcamera.GetViewMatrix();

//        for (GLuint i = 0; i < lightPositions.size(); i++)
        {
            glUniform3fv(glGetUniformLocation(shaderShadow.Program, ("LightPosition")), 1, glm::value_ptr(glm::vec3((8.0*cos(m_lightAngle), 4.0 ,8.0*sin(m_lightAngle)))));
            glUniform4fv(glGetUniformLocation(shaderShadow.Program, ("inColour" )), 1, glm::value_ptr(glm::vec4(1,1,1,0)));
        }

        glm::mat4 bias;
        glm::scale(bias, glm::vec3(0.5f,0.5f,0.5f));
        glm::translate(bias, glm::vec3(0.5f,0.5f,0.5f));
        glm::mat4 textureMatrix = bias*projection*view*model;
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "textureMatrix"), 1, GL_FALSE, glm::value_ptr(textureMatrix));


      view = camera.GetViewMatrix();

        glUniform3fv(glGetUniformLocation(shaderShadow.Program, "viewPos"), 1, &camera.Position[0]);
//         - create one large cube that acts as the floor
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0));
        model = glm::scale(model, glm::vec3(25.0f, 1.0f, 25.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "MV"), 1, GL_FALSE, glm::value_ptr(view*model));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "MVP"), 1, GL_FALSE, glm::value_ptr(projection*view*model));
        RenderCube();

        glActiveTexture(GL_TEXTURE0);


//        //working showing floor cube
//        shader.Use();
//                    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
//                    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
//                    glActiveTexture(GL_TEXTURE0);
//                    glBindTexture(GL_TEXTURE_2D, woodTexture);
//                    // - set lighting uniforms
//                    for (GLuint i = 0; i < lightPositions.size(); i++)
//                    {
//                        glUniform3fv(glGetUniformLocation(shader.Program, ("lights[" + std::to_string(i) + "].Position").c_str()), 1, &lightPositions[i][0]);
//                        glUniform3fv(glGetUniformLocation(shader.Program, ("lights[" + std::to_string(i) + "].Color").c_str()), 1, &lightColors[i][0]);
//                    }

//                    glUniform3fv(glGetUniformLocation(shader.Program, "viewPos"), 1, &camera.Position[0]);
//                    // - create one large cube that acts as the floor
//                    model = glm::mat4();
//                    model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0));
//                    model = glm::scale(model, glm::vec3(25.0f, 1.0f, 25.0f));
//                    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
//                    RenderCube();


        //working showing floor cube
        // - then create multiple cubes as the scenery
        glBindTexture(GL_TEXTURE_2D, containerTexture);
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "MV"), 1, GL_FALSE, glm::value_ptr(view*model));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "MVP"), 1, GL_FALSE, glm::value_ptr(projection*view*model));
        RenderCube();
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "MV"), 1, GL_FALSE, glm::value_ptr(view*model));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "MVP"), 1, GL_FALSE, glm::value_ptr(projection*view*model));
        RenderCube();
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(-1.0f, -1.0f, 2.0));
        model = glm::rotate(model, 60.0f, glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(2.0));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "MV"), 1, GL_FALSE, glm::value_ptr(view*model));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "MVP"), 1, GL_FALSE, glm::value_ptr(projection*view*model));
        RenderCube();
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(0.0f, 2.7f, 4.0));
        model = glm::rotate(model, 23.0f, glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(2.5));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "MV"), 1, GL_FALSE, glm::value_ptr(view*model));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "MVP"), 1, GL_FALSE, glm::value_ptr(projection*view*model));
        RenderCube();
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -3.0));
        model = glm::rotate(model, 124.0f, glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(2.0));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "MV"), 1, GL_FALSE, glm::value_ptr(view*model));
        glUniformMatrix4fv(glGetUniformLocation(shaderShadow.Program, "MVP"), 1, GL_FALSE, glm::value_ptr(projection*view*model));
        RenderCube();
//        RenderCube();
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
//        RenderCube();
        // - finally show all the light sources as bright cubes
//        shaderLight.Use();
//        glUniformMatrix4fv(glGetUniformLocation(shaderLight.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
//        glUniformMatrix4fv(glGetUniformLocation(shaderLight.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

//        for (GLuint i = 0; i < lightPositions.size(); i++)
//        {
//            model = glm::mat4();
//            model = glm::translate(model, glm::vec3(lightPositions[i]));
//            model = glm::scale(model, glm::vec3(0.5f));
//            glUniformMatrix4fv(glGetUniformLocation(shaderLight.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
//            glUniform3fv(glGetUniformLocation(shaderLight.Program, "lightColor"), 1, &lightColors[i][0]);
//            RenderCube();
//        }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);







//        // 2. Blur bright fragments w/ two-pass Gaussian Blur
//        GLboolean horizontal = true, first_iteration = true;
//        GLuint amount = 10;
//        shaderBlur.Use();
//        for (GLuint i = 0; i < amount; i++)
//        {

//#if USE_CANCEL_OUT_testpingpong2_shader

//            if(!horizontal)
//            {
//                testpingpong2.Use();
//            }
//            else
//            {
//                shaderBlur.Use();
//            }
//#endif
//            //at first iteration bind pingpongFBO[1], then bind pingpongFBO[0], then bind pingpongFBO[1]
//            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
//            glUniform1i(glGetUniformLocation(shaderBlur.Program, "horizontal"), horizontal);

//            // bind texture of other framebuffer (or scene if first iteration)

//            //i)at first bind 2nd pingpongFBO[1] bind brightness coming out of layout (location = 1) out vec4 BrightColor; --> hence use "colorBuffers[1]"
//            //ii)next bind 1st pingpongFBO[0] bind what's  coming out of layout the other framebuffer;


//            //at first USE colorBuffers[1] (because the brightness is attached to the layout 1), then the texture of pingpongColorbuffers[1], then bind pingpongColorbuffers[0]
//            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);

//            //colorBuffers[0] is the FragColor of bloom.frag
//            //colorBuffers[1] is the BrightColor of bloom.frag (TRY REPLACING colorBuffers[1] WITH colorBuffers[0]) AT glBindTexture CALL AND THE WHOLE SCENE WOULD BE BLURRED
//            // pingpongColorbuffers[0] is 1 extra buffer for rendering
//            // pingpongColorbuffers[1] is a 2nd extra buffer for rendering

////            glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
////            glBindTexture(GL_TEXTURE_2D, colorBuffers[1]);
////            glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[0]);
////            glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[1]);


//            RenderQuad();
//            horizontal = !horizontal;
//            if (first_iteration)
//                first_iteration = false;
//        }
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);

//        // 2. Now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        shaderBloomFinal.Use();
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);//bind the actual scene not the blurred stuff
////        //glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[0]);//use one of the 2 pingpongColorbuffers, as each one was outputing its result back to the other
//        glUniform1i(glGetUniformLocation(shaderBloomFinal.Program, "bloom"), bloom);
//        glUniform1f(glGetUniformLocation(shaderBloomFinal.Program, "exposure"), exposure);
//        RenderQuad();


		// Swap the buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}


// RenderQuad() Renders a 1x1 quad in NDC, best used for framebuffer color targets
// and post-processing effects.
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
	if (quadVAO == 0)
	{
		GLfloat quadVertices[] = {
			// Positions        // Texture Coords
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// Setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// RenderCube() Renders a 1x1 3D cube in NDC.
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
void RenderCube()
{
	// Initialize (if necessary)
	if (cubeVAO == 0)
	{
		GLfloat vertices[] = {
			// Back face
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,  // top-right
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
			-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,// top-left
			// Front face
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
			0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // top-right
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
			-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom-left
			// Left face
			-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
			-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
			-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
			-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
			// Right face
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
			0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right         
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-right
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // top-left
			0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left     
			// Bottom face
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
			0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,// bottom-left
			0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			// Top face
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right     
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
			-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// Fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// Link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// Render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// This function loads a texture from file. Note: texture loading functions like these are usually 
// managed by a 'Resource Manager' that manages all resources (like textures, models, audio). 
// For learning purposes we'll just define it as a utility function.
GLuint loadTexture(GLchar const * path)
{
	// Generate texture ID and load texture data 
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textureID;

}

bool keys[1024];
bool keysPressed[1024];
// Moves/alters the camera positions based on user input
void Do_Movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (keys[GLFW_KEY_SPACE] && !keysPressed[GLFW_KEY_SPACE])
	{
        bloom = !bloom;
		keysPressed[GLFW_KEY_SPACE] = true;
	}

	// Change parallax height scale
	if (keys[GLFW_KEY_Q])
		exposure -= 0.5 * deltaTime;
	else if (keys[GLFW_KEY_E])
		exposure += 0.5 * deltaTime;
}

GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key <= 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
			keysPressed[key] = false;
		}
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}











glViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());
glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
shader->use("GeometryTesellation");
         m_transform.reset();
         m_transform.setPosition(0.0f,1.0f,0.0f);
         m_vaoMesh->bind();
         loadToGeometryMatrices();
         m_vaoMesh->draw();
 glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

  // 2. generate SSAO texture
  // --------------------------------------------------------------------------------
glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());
shader->use("ssaoShader");
 GLint pidSSAO = shader->getProgramID("ssaoShader");
 // Send kernel + rotation
 for (unsigned int i = 0; i < 64; ++i)
 {
     setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
         //glUniform3fv( glGetUniformLocation(pidSSAO, ( "samples["+ std::to_string(i)+"]").c_str() ), ssaoKernel.size(),reinterpret_cast<GLfloat *>(&ssaoKernel[0]));
 }
              glActiveTexture(GL_TEXTURE10);
              glBindTexture(GL_TEXTURE_2D, gPosition);
              glGenerateMipmap(GL_TEXTURE_2D);
              glActiveTexture(GL_TEXTURE11);
              glBindTexture(GL_TEXTURE_2D, gNormal);
               glGenerateMipmap(GL_TEXTURE_2D);
              glActiveTexture(GL_TEXTURE12);
              glBindTexture(GL_TEXTURE_2D, noiseTexture);
               glGenerateMipmap(GL_TEXTURE_2D);
              loadMatricesToShader();
              prim->draw("SSAOplane");
 glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
 // 3. blur SSAO texture to remove noise
     // ------------------------------------
glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
     glClear(GL_COLOR_BUFFER_BIT);
     glViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());
shader->use("blur");
                 glActiveTexture(GL_TEXTURE10);
                 glBindTexture(GL_TEXTURE_2D, ssaoBlurFBO);
                 glGenerateMipmap(GL_TEXTURE_2D);
                 glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
                 loadMatricesBASE();
                 prim->draw("SSAOplane");
 glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
     // 4. lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
     // -----------------------------------------------------------------------------------------------------
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     glViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());
shader->use("lightShader");
     ngl::Vec3 lightColor = ngl::Vec3(0.2, 0.2, 0.7);
     ngl::Vec3 lightPosView = ngl::Mat3(m_cam.getViewMatrix()) * ngl::Vec3(m_lightPosition);
     shader->setUniform("light.Position", lightPosView);
     shader->setUniform("light.Color", lightColor);
     // Update attenuation parameters
     const float linear    = 0.09;
     const float quadratic = 0.032;
     shader->setUniform("light.Linear",linear );
     shader->setUniform("light.Quadratic",quadratic );
     glActiveTexture(GL_TEXTURE10);
     glBindTexture(GL_TEXTURE_2D, gPosition);
     glGenerateMipmap(GL_TEXTURE_2D);
     glActiveTexture(GL_TEXTURE11);
          glGenerateMipmap(GL_TEXTURE_2D);
     glBindTexture(GL_TEXTURE_2D, gNormal);
         glGenerateMipmap(GL_TEXTURE_2D);
     glActiveTexture(GL_TEXTURE12);
     glBindTexture(GL_TEXTURE_2D, gAlbedo);
         glGenerateMipmap(GL_TEXTURE_2D);
     glActiveTexture(GL_TEXTURE14); // add extra SSAO texture to lighting pass
     glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
     loadMatricesToShader();
     prim->draw("SSAOplane");
 glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
 glBindTexture(GL_TEXTURE_2D, 0);
 m_vaoMesh->unbind();
























