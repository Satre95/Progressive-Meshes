#pragma once

#include "Mesh.hpp"

namespace starforge {
	class Cube: public Mesh
	{
	public:
		Cube( RenderDevice & renderDevice, float size = 1.0f);
		~Cube();

	private:
		struct CubeVertex {
			CubeVertex(float xIn, float yIn, float zIn,
						float nXIn, float nYIn, float nZIn,
						float uIn, float vIn) :
					x(xIn), y(yIn), z(zIn),
					nx(nXIn), ny(nYIn), nz(nZIn),
					u(uIn), v(vIn) {}
			float x, y, z;
			float nx, ny, nz;
			float u, v;

			static size_t NumComponents() { return 8;}
		};
		// (Pos + Normal + TexCoord) * (3 * # Triangles)
		// (3 + 3 + 2) * (3 * 12) = 288
		static CubeVertex unitCubeVerts[36];
		
	};
}