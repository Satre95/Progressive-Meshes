#include "DefaultShaders.hpp"
namespace starforge
{
	//--------------------------------------------------------------------
#ifdef __APPLE__
	const char * g_defaultVertexShaderSource = R"END(
#version 410 core
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uArcball;
uniform mat3 uNormalMatrix;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColor;

out vec3 FragPos;
out vec3 FragNormal;
out vec4 FragVertColor;

void main()
{
gl_Position = uProjection * uView * uArcball * uModel  * vec4(aPos, 1.0f);
FragPos = vec3(gl_Position);
FragNormal = mat3(transpose(inverse(uModel))) * aNormal;
FragVertColor = aColor;
}
	)END";

#else
	const char * g_defaultVertexShaderSource = R"END(
#version 450 core
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uArcball;
uniform mat3 uNormalMatrix;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColor;

out vec3 FragPos;
out vec3 FragNormal;
out vec4 FragVertColor;

void main()
{
gl_Position = uProjection * uView * uArcball * uModel  * vec4(aPos, 1.0f);
FragPos = vec3(gl_Position);
FragNormal = mat3(transpose(inverse(uModel))) * aNormal;
FragVertColor = aColor;
}
	)END";
#endif

	//--------------------------------------------------------------------
#ifdef __APPLE__
	const char * g_defaultPixelShaderSource = R"END(
struct DirectLight
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
};

uniform DirectLight dLight1 = DirectLight(
								normalize(vec3(0.9f, -0.5f, -1.f)),
								vec3(0.2f),
								vec3(1.f)
								);
uniform DirectLight dLight2 = DirectLight(
								normalize(vec3(-0.9f, -0.5f, 1.f)),
								vec3(0.2f),
								vec3(1.f, 0.8f, 0.8f)
								);

uniform vec3 uDefaultFragColor;
uniform bool uComputeShading;

in vec3 FragPos;
in vec3 FragNormal;
in vec4 FragVertColor;

out vec4 FragColor;

void main()
{
	if(uComputeShading) {
		// Ambient
		const float ambientStrength = 0.2f;
		vec3 ambient1 = ambientStrength * dLight1.ambient;
		vec3 ambient2 = ambientStrength * dLight2.ambient;

		// Diffuse
		vec3 norm = normalize(FragNormal);
		vec3 lightDir = normalize(-dLight1.direction);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse1 = diff * dLight1.diffuse;

		lightDir = normalize(-dLight2.direction);
		diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse2 = diff * dLight2.diffuse;

		FragColor = vec4((ambient1 + ambient2 + diffuse1 + diffuse2) * uDefaultFragColor, 1.f);
	}
	else
	{
		FragColor = vec4(uDefaultFragColor, 1.f);
	}
}
	)END";

#else
	const char * g_defaultPixelShaderSource = R"END(
#version 450 core
struct DirectLight
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
};

uniform DirectLight dLight1 = DirectLight(
								normalize(vec3(0.9f, -0.5f, -1.f)),
								vec3(0.2f),
								vec3(1.f)
								);
uniform DirectLight dLight2 = DirectLight(
								normalize(vec3(-0.9f, -0.5f, 1.f)),
								vec3(0.2f),
								vec3(1.f, 0.8f, 0.8f)
								);

uniform vec3 uDefaultFragColor;
uniform bool uComputeShading;

in vec3 FragPos;
in vec3 FragNormal;
in vec4 FragVertColor;

out vec4 FragColor;

void main()
{
	if(uComputeShading) {
		// Ambient
		const float ambientStrength = 0.2f;
		vec3 ambient1 = ambientStrength * dLight1.ambient;
		vec3 ambient2 = ambientStrength * dLight2.ambient;

		// Diffuse
		vec3 norm = normalize(FragNormal);
		vec3 lightDir = normalize(-dLight1.direction);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse1 = diff * dLight1.diffuse;

		lightDir = normalize(-dLight2.direction);
		diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse2 = diff * dLight2.diffuse;

		FragColor = vec4((ambient1 + ambient2 + diffuse1 + diffuse2) * uDefaultFragColor, 1.f);
	}
	else
	{
		FragColor = vec4(uDefaultFragColor, 1.f);
	}
}
	)END";
#endif
}
