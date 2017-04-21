#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D image;
uniform bool horizontal;

uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{
     vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
     vec3 result = texture(image, TexCoords).rgb * weight[0];


//Modify this in any way and the previous buffer "pinged" (probably using another shader) will be modified and "ponged" over to the other buffer
//do this blurring 10 times more and each ping..or.. pong
//    for(int i = 1; i < 10; ++i)
    {
        if(horizontal)
         {
             for(int i = 1; i < 5; ++i)
             {
                //THE -= here will cancel the previous buffer blurred result

                result -= texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
                result -= texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
             }
         }
         else
         {

               //THE -= here will cancel the previous buffer blurred result

             for(int i = 1; i < 5; ++i)
             {
                 result -= texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
                 result -= texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
             }
         }
     }

     FragColor = vec4(result, 1.0);
}
