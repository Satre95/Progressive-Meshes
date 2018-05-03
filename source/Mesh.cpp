#include "Mesh.hpp"
#include "Platform.hpp"

namespace starforge
{
	Mesh::Mesh() {}

	Mesh::Mesh(std::vector <float>& vertsIn,
		std::vector<unsigned int>& indicesIn,
		VertexDescription * vdIn) : m_vertexDescription(vdIn)
	{
		m_vertices = vertsIn;
		m_indices = indicesIn;
	}

	Mesh::~Mesh()
	{
		delete m_ebo; delete m_vbo;
		delete m_vao; delete m_vertexDescription;
	}

	void Mesh::InitBuffers(RenderDevice & renderDevice)
	{
		m_vbo = renderDevice.CreateVertexBuffer(m_vertices.size(), m_vertices.data());
		if(HasIndices())
			m_ebo = renderDevice.CreateIndexBuffer(m_indices.size(), m_indices.data());

		m_vao = renderDevice.CreateVertexArray(1, &m_vbo, &m_vertexDescription);
	}
}