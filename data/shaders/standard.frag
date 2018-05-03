#version 400 core

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
uniform bool uComputeShading = true;

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