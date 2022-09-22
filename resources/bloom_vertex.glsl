#version 330 core
layout (location = 0) in vec2 vertPos;
layout (location = 1) in vec2 vertTex;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(vertPos.x, vertPos.y, 0.0, 1.0); 
    TexCoords = vertTex;
}  