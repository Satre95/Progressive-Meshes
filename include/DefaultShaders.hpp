#pragma once

namespace starforge {
	/**
	 * The default vertex shader.
	 *
	 * Supports vertices with attributes with binding locations in the following order:
	 *  0. position
	 *  1. normal
	 *  2. texture coordinate
	 *  3. tangent
	 *  4. bitangent
	 *  5. color
	 */
	extern const char * g_defaultVertexShaderSource;

	/**
	 * The default fragment shader.
	 *
	 * Supports up to 4 textures:
	 *  - uTextureDiffuse
	 *  - uTextureSpecular
	 *  - uTextureNormal
	 *  - uTextureHeight
	 *
	 * First, diffuse & specular textures are checked. If not provided, then
	 * vertex color is checked. If that is also not provided, then a default
	 * color of blue-green is used. If a normal map is provided, then that is
	 * used over the vertex normal attribute.
	*/
	extern const char * g_defaultPixelShaderSource;
}