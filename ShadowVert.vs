
#version 330 core

/*
//#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    vs_out.FragPos = vec3(model * vec4(position, 1.0));
    vs_out.TexCoords = texCoords;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vs_out.Normal = normalize(normalMatrix * normal);
}

*/

/// modified from the OpenGL Shading Language Example "Orange Book"
/// Roost 2002

uniform mat4 MV;
uniform mat4 MVP;
uniform mat3 normalMatrix;
uniform mat4 textureMatrix;
uniform vec3 LightPosition;
uniform  vec4  inColour;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;


uniform mat4 model;



layout (location=0) in  vec4  inVert;
layout (location=2) in vec2 inUV;
layout (location=1) in  vec3  inNormal;
out vec4  ShadowCoord;
out vec4  Colour;
void main()
{

vec4 ecPosition = MV * inVert;
vec3 ecPosition3 = (vec3(ecPosition)) / ecPosition.w;
vec3 VP = LightPosition - ecPosition3;
VP = normalize(VP);
vec3 normal = normalize(normalMatrix * inNormal);
float diffuse = max(0.0, dot(normal, VP));
vec4 texCoord = textureMatrix * inVert;
ShadowCoord   = texCoord;
Colour  = vec4(diffuse*inColour.rgb,inColour.a);
gl_Position    = MVP * inVert;



vs_out.FragPos = vec3(model * inVert);
vs_out.TexCoords = inUV;

mat3 normalMatrix = transpose(inverse(mat3(model)));
vs_out.Normal = normalize(normalMatrix * normal);
}

