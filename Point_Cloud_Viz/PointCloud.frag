#version 330 core

in vec3 normalColor; // The input variable from the vertex shader (same name and same type)

out vec4 color;

void main()
{
    color = vec4(normalColor, 1.0f);
}
