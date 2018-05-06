#include "ProgMesh.hpp"
#include <utility>

size_t Vertex::sCount = 0;
size_t Face::sCount = 0;

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

void ProgMesh::BuildConnectivity() {
    // Clear any previous adjacency
    mVertexFaceAdjacency.clear();
    mVertexFaceAdjacency = std::unordered_multimap<const Vertex *, const Face *, VertexPtrHash>();

    // Iterate over all faces and add to vertex to Face adjacency list.
    for(Face & aFace: mFaces) {
        for (int i = 0; i < 3; ++i) {

            Vertex * aVertex = &(mVertices.at(aFace.GetIndex(i)));
            mVertexFaceAdjacency.insert(std::make_pair(aVertex, &aFace));
        }
    }
}

void ProgMesh::PrintConnectivity(std::ostream & os) const {
    for(const Vertex & aVertex: mVertices) {
        auto range = mVertexFaceAdjacency.equal_range(&aVertex);
        size_t count = 0;
        for(auto it = range.first; it != range.second; ++it) {
            auto vert = it->first;
            auto face = it->second;
            count++;
        }
        os << "\t\tVertex " << aVertex.mId << " is adjacent to " << count << " faces." << std::endl;
    }
}