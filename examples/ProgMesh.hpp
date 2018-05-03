#pragma once

#include <vector>
#include "Geometry.hpp"

/**
 * This class represents geometry in space and any associated transformations on that geometry.
 */
class ProgMesh
{
public:
	ProgMesh() = default;
	ProgMesh(std::vector<Vertex> & _verts, std::vector<Face> & _faces);
	
private:

	/// The vertices that compose this ProgMesh
	std::vector<Vertex> mVertices;

	/// Stores the faces. Faces are indices into the vertex array.
	std::vector<Face> mFaces;

	friend class ProgModel;
};