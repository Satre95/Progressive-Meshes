#version 400 core
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uArcball;
uniform mat3 uNormalMatrix;

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec4 aColor;

out vec3 FragPos;
out vec3 FragNormal;
out vec4 FragVertColor;

void main()
{
    gl_Position = uProjection * uView * uArcball * uModel  * aPos;
    FragPos = vec3(gl_Position);
    FragNormal = uNormalMatrix * vec3(aNormal);
    FragVertColor = aColor;
}