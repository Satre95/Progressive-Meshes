#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "RenderDevice.hpp"

namespace starforge
{
	class Mesh
	{
	public:
		friend class Model;
		Mesh(std::vector <float> &, std::vector<unsigned int> &, VertexDescription *);
		~Mesh();

		size_t NumVertices() const { return m_numVertices; }
		size_t NumIndices() const { return m_indices.size(); }
		size_t NumTextures() const { return m_textures.size(); }
		Texture2D * GetTexture(size_t i) const { return m_textures.at(i); }
		VertexBuffer * GetVertexBuffer() const { return m_vbo; }
		VertexArray * GetVertexArray() const { return m_vao; }
		IndexBuffer * GetIndexBuffer() const { return m_ebo; }
		bool HasIndices() { return m_indices.size() != 0; }

		/// Allocates the buffers and GPU mem.
		void InitBuffers(RenderDevice & renderdevice);
		///Draws the mesh. Expects the pipeline view and projection matrices to already be set.
	protected:
		Mesh(); //Direct creation is disallowed.

		/**
		 * The vertices of this mesh are stored as a generic array of floats,
		 * the element description is used at runtime to specify the attributes.
		 */
		std::vector <float> m_vertices;
		/// Indices of this mesh.
		std::vector <unsigned int> m_indices;
		/// The textures used by this mesh
		std::vector <Texture2D *> m_textures;
		/// Describes the information on each vertex
		VertexDescription * m_vertexDescription;
		/// Encapsulation of the VBO in GPU mem.
		VertexBuffer *m_vbo = nullptr;
		/// Encapsulation of the VAO in GPU mem
		VertexArray *m_vao = nullptr;
		/// Encapsulation of the EBO in GPU mem
		IndexBuffer *m_ebo = nullptr;
		unsigned int m_numVertices = 0;
	};
}