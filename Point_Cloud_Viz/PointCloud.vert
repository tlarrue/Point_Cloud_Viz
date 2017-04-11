#version 330 core

uniform mat4 ProjectionMatrix, ViewMatrix, ModelMatrix, transform;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 normalColor;
//out float dist;

void main()
{
    //gl_Position = vec4(position, 1.0);
    gl_Position = transform * ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(position, 1.f);
    //ourColor = color;
    normalColor = vec3(color.r/256.0, color.g/256.0, color.b/256.0);
    gl_PointSize=2;
    
    
    //clr = vec3(0.5f, 0.0f, 0.0f); // Set the output variable to a dark-red color
    //clr = vec3(vertexColor.r/255.0, vertexColor.g/255.0, vertexColor.b/255.0);
    //clr = vec4(vertexColor.bgr, 1.f);
    //dist = sqrt(dot(vertexPosition, vertexPosition));
}
