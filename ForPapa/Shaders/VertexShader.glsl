#version 400 core

in vec3 in_position;
in vec3 in_normal;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform vec3 lightDir;
uniform vec3 lightRad;

flat out vec3 radiance;

void main(void)
{
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    
    gl_Position = projectionMatrix*viewMatrix*modelMatrix*vec4(in_position,1.0f);

    vec3 normal = normalMatrix*in_normal;
    radiance = lightRad*dot(normal,lightDir);
}
