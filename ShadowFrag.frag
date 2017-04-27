#version 330 core

/// modified from the OpenGL Shading Language Example "Orange Book"
/// Roost 2002


in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Light {
    vec3 Position;
    vec3 Color;
};


uniform Light lights[4];
uniform sampler2D diffuseTexture;
uniform vec3 viewPos;


in vec4 Colour;
layout (location=0) out vec4 outColour;
layout (location = 1) out vec4 BrightColor;

// for mac we need this
//uniform sampler2DShadow ShadowMap;
// for windows unix we need
uniform sampler2D ShadowMap;

in vec4 ShadowCoord;


void main ()
{
  float shadeFactor=textureProj(ShadowMap,ShadowCoord).x;
        shadeFactor=shadeFactor;// * 0.25 + 0.75;
  //outColour=vec4( shadeFactor * Colour.rgb, Colour.a);
  //outColour=vec4(ShadowCoord);


  vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
  vec3 normal = normalize(fs_in.Normal);
  // Ambient
  vec3 ambient = 1.0 * color;
  // Lighting
  vec3 lighting = vec3(0.0f);
  vec3 viewDir = normalize(viewPos - fs_in.FragPos);
  for(int i = 0; i < 4; i++)
  {
      // Diffuse
      vec3 lightDir = normalize(lights[i].Position - fs_in.FragPos);
      float diff = max(dot(lightDir, normal), 0.0);
      vec3 result = lights[i].Color * diff * color;
      // Attenuation (use quadratic as we have gamma correction)
      float distance = length(fs_in.FragPos - lights[i].Position);
      result *= 1.0 / (distance * distance);
      lighting += result;

  }
  vec3 result = ambient + lighting;
  // Check whether result is higher than some threshold, if so, output as bloom threshold color
  float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));

  //!! ONLY THE BRIGHT FRAGMENTS WILL BE EVALUATED (BrightColor will be set) FOR BLURRING !!
  if(brightness > 1.0)
      BrightColor = vec4(result, 1.0);
  // else
      // BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
  outColour = vec4(/*shadeFactor**/result, 1.0f);


}
