#pragma once

#include <glm/matrix.hpp>
#include <unordered_map>
#include <functional>
#include <vector>
#include "Geometry.hpp"
#include "RenderDevice.hpp"

/**
 * This class represents geometry in space and any associated transformations on that geometry.
 */
class ProgMesh
{
public:
	ProgMesh() = default;
	ProgMesh(std::vector<Vertex> & _verts, std::vector<Face> & _faces);
    ProgMesh(std::vector<Vertex> & _verts, std::vector<uint32_t > & _indices);

	const glm::mat4 & GetModelMatrix() const { return  mModelMatrix; }
	glm::mat4 & GetModelMatrix() { return mModelMatrix; }

	void AllocateBuffers(starforge::RenderDevice & renderDevice);
    void Draw(starforge::RenderDevice & renderDevice);
private:
	/// The vertices that compose this ProgMesh
	std::vector<Vertex> mVertices;

	/// Stores the faces. Faces are indices into the vertex array.
	std::vector<Face> mFaces;
	std::vector<uint32_t> mIndices;

	/// The vertex to face adjacency.
	std::unordered_multimap<Vertex*, Face*, VertexPtrHash> mVertexFaceAdjacency;

	glm::mat4 mModelMatrix;

	/// The GPU buffers are managed by the render device.
	/// If you want to delete them manually, you must do it through one of the
	/// associated Delete____Buffer methods.
	/// DO NOT call delete buf at any time.
	starforge::VertexArray * mVAO = nullptr;
	starforge::VertexBuffer * mVBO = nullptr;
	starforge::IndexBuffer * mIBO = nullptr;
    starforge::VertexDescription * mVertexDescription = nullptr;
	friend class ProgModel;
};