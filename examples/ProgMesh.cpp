#include "ProgMesh.hpp"
#include <utility>
#include <algorithm>

size_t Vertex::sCount = 0;
size_t Face::sCount = 0;

ProgMesh::ProgMesh(std::vector<Vertex> & _verts, std::vector<Face> & _faces):
mVertices(_verts), mFaces(_faces)
{
    mIndices.reserve(mFaces.size() * 3);
    for(auto & aFace: mFaces) {
        mIndices.push_back(std::find(mVertices.begin(), mVertices.end(), *aFace.GetVertex(0)) - mVertices.begin());
        mIndices.push_back(std::find(mVertices.begin(), mVertices.end(), *aFace.GetVertex(1)) - mVertices.begin());
        mIndices.push_back(std::find(mVertices.begin(), mVertices.end(), *aFace.GetVertex(2)) - mVertices.begin());

    }
}

ProgMesh::ProgMesh(std::vector<Vertex> & _verts, std::vector<uint32_t > & _indices) :
mVertices(_verts), mIndices(_indices) {
    mFaces.reserve(_indices.size() / 3);
    for (int i = 0; i < _indices.size(); i+=3)
        mFaces.emplace_back(mVertices.at(_indices.at(i)), mVertices.at(_indices.at(i+1)), mVertices.at(_indices.at(i+2)));
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
    mVertexFaceAdjacency = std::unordered_multimap<Vertex *, Face *, VertexPtrHash>();
    mEdges.clear();
    mEdges = std::unordered_multimap<Vertex *, Vertex *, VertexPtrHash>();

    // Iterate over all faces and add to vertex to Face adjacency list.
    for(Face & aFace: mFaces) {
        for (int i = 0; i < 3; ++i) {
            Vertex * aVertex = aFace.GetVertex(i);
            mVertexFaceAdjacency.insert(std::make_pair(aVertex, &aFace));
        }
    }

    // Go over each face and add the edges to the vertex to vertex adjacency list.  TODO: as it is now, each interior pair will be added twice
    for(Face & aFace: mFaces) {
        Vertex * v0 = aFace.GetVertex(0);
        Vertex * v1 = aFace.GetVertex(1);
        Vertex * v2 = aFace.GetVertex(2);

        // Add v1 and v2 for v0
        mEdges.insert(std::make_pair(v0, v1));
        mEdges.insert(std::make_pair(v0, v2));

        // Add v0 and v2 for v1
        mEdges.insert(std::make_pair(v1, v0));
        mEdges.insert(std::make_pair(v1, v2));

        // Add v0 and v1 for v2
        mEdges.insert(std::make_pair(v2, v0));
        mEdges.insert(std::make_pair(v2, v1));
    }
}

void ProgMesh::PrintConnectivity(std::ostream & os) const {
//    for(const Vertex & aVertex: mVertices) {
//        auto range = mVertexFaceAdjacency.equal_range(&aVertex);
//        size_t count = 0;
//        for(auto it = range.first; it != range.second; ++it) {
//            auto vert = it->first;
//            auto face = it->second;
//            count++;
//        }
//        os << "\t\tVertex " << aVertex.mId << " is adjacent to " << count << " faces." << std::endl;
//    }

    os << "\t\tThere are " << mEdges.size() << " edges in this mesh." << std::endl;
}

std::vector<Vertex *> ProgMesh::GetConnectedVertices(Vertex * aVertex) const {
    auto range = mEdges.equal_range(aVertex);
    std::vector<Vertex *> neighbors;
    for(auto it = range.first; it != range.second; ++it) {
        neighbors.push_back(it->second);
    }
    return neighbors;
}

std::vector<Face *> ProgMesh::GetAdjacentFaces(Vertex * aVertex) const {
    auto range = mVertexFaceAdjacency.equal_range(aVertex);
    std::vector<Face*> neighbors;
    for(auto it = range.first; it != range.second; ++it) {
        neighbors.push_back(it->second);
    }
    return neighbors;
}

glm::mat4 ProgMesh::ComputeQuadric(Vertex * aVertex) const {

	std::vector<Face* > faces = GetAdjacentFaces(aVertex);
	glm::vec3 v0, v1, v2, n;
	glm::vec4 q;
	glm::mat4 Q = glm::mat4(0.0f);

	for (Face* & aFace : faces) {
		v0 = aFace->GetVertex(0)->mPos;
		v1 = aFace->GetVertex(1)->mPos;
		v2 = aFace->GetVertex(2)->mPos;

		n = glm::cross(v1 - v0, v2 - v0);
		n = n * (1 / glm::length(n));

		q = { n.x,n.y,n.z,glm::dot(-n,v0) };
		Q += q * q;
	}
	return Q;
}

void ProgMesh::PreparePairs() {

	std::vector<Vertex*> neighbors;
	Pair* newPair;
	Vertex* vOptimal;
	float error;
	
	for (Vertex & aVertex : mVertices) {
		// Compute quadric for each vertex
		mQuadrics.insert(std::make_pair(&aVertex, ComputeQuadric(&aVertex)));

		// Compute error for each pair and order them
		neighbors = GetConnectedVertices(&aVertex);
		for (Vertex* & aNeighbor : neighbors) {
			newPair = new Pair(&aVertex, aNeighbor);

			// only midpoint TODO - can definetly make this the legit optimal w/o too much trouble
			vOptimal = newPair->CalcOptimal();
			error = glm::dot(vOptimal->mPos, 
					(mQuadrics[&aVertex] + mQuadrics[aNeighbor]) * vOptimal->mPos);

			mPairs.insert(std::make_pair(error, newPair));
		}
	}
}