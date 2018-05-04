#include "ProgMesh.hpp"

ProgMesh::ProgMesh(std::vector<Vertex> & _verts, std::vector<Face> & _faces):
mVertices(_verts), mFaces(_faces)
{
    mIndices.reserve(mFaces.size() * 3);
    for(auto & aFace: mFaces) {
        mIndices.push_back(aFace.GetIndex(0));
        mIndices.push_back(aFace.GetIndex(1));
        mIndices.push_back(aFace.GetIndex(2));
    }
}

ProgMesh::ProgMesh(std::vector<Vertex> & _verts, std::vector<uint32_t > & _indices) :
mVertices(_verts), mIndices(_indices) {
    mFaces.reserve(_indices.size() / 3);
    for (int i = 0; i < _indices.size(); i+=3)
        mFaces.emplace_back(_indices.at(i), _indices.at(i+1), _indices.at(i+2));
}

void ProgMesh::AllocateBuffers(starforge::RenderDevice &renderDevice) {
    if(mVAO) renderDevice.DestroyVertexArray(mVAO);
    if(mVBO) renderDevice.DestroyVertexBuffer(mVBO);
    if(mIBO) renderDevice.DestroyIndexBuffer(mIBO);
    if(mVertexDescription) renderDevice.DestroyVertexDescription(mVertexDescription);

    mVBO = renderDevice.CreateVertexBuffer(mVertices.size() * sizeof(Vertex), mVertices.data());
    mIBO = renderDevice.CreateIndexBuffer(mIndices.size() * sizeof(uint32_t), mIndices.data());
    starforge::VertexElement vertexElements[] = {
            {0, starforge::VERTEXELEMENTTYPE_FLOAT, 4, sizeof(Vertex), 0}, // Position attribute
            {1, starforge::VERTEXELEMENTTYPE_FLOAT, 4, sizeof(Vertex), sizeof(glm::vec4)}, // Normal attribute
            {2, starforge::VERTEXELEMENTTYPE_FLOAT, 4, sizeof(Vertex), sizeof(glm::vec4) * 2} // Color attribute.
    };
    mVertexDescription = renderDevice.CreateVertexDescription(3, vertexElements);

    mVAO = renderDevice.CreateVertexArray(1, &mVBO, &mVertexDescription);
}

void ProgMesh::Draw(starforge::RenderDevice &renderDevice) {
    renderDevice.SetVertexArray(mVAO);
    renderDevice.SetIndexBuffer(mIBO);

    renderDevice.DrawTrianglesIndexed32(0, (int)mIndices.size());
}