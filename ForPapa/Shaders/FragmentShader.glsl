#version 420 core

flat in vec3 radiance;
out vec4 fragColor;

vec4 tonemap(vec3 rad)
{
    for (int i=0; i<3; i++)
    {
        rad[i] /= (1.0f + rad[i]);
        rad[i] = pow(rad[i] , 1.0f/2.2f);
    }
    return vec4(rad, 1.0f);
}

void main(void)
{
    fragColor = tonemap(radiance);
}