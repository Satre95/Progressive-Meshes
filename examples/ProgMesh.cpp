#include "ProgMesh.hpp"
#include "Utilities.hpp"
#include <utility>
#include <algorithm>

size_t Vertex::sCount = 0;
size_t Face::sCount = 0;

//ProgMesh::ProgMesh(std::vector<Vertex> & _verts, std::unordered_set<Face> & _faces):
//mVertices(_verts)
//{
//	mFaces.reserve(_faces.size());
//	for (auto faceItr = _faces.begin(); faceItr != _faces.begin(); faceItr++) {
//		mFaces.insert(new Face(*faceItr));
//	}
//
//	GenerateIndicesFromFaces();
//}

ProgMesh::ProgMesh(std::vector<Vertex> & _verts, std::vector<uint32_t > & _indices) :
mVertices(_verts), mIndices(_indices) {
	for (int i = 0; i < _indices.size(); i+=3) {
		mFaces.insert(new Face(mVertices.at(_indices.at(i)), mVertices.at(_indices.at(i+1)), mVertices.at(_indices.at(i+2))));
	}
}

ProgMesh::~ProgMesh() {
	for(auto & facePtr: mFaces) {
		delete facePtr;
	}
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
	for(auto & aFace: mFaces) {
		for (int i = 0; i < 3; ++i) {
			Vertex * aVertex = aFace->GetVertex(i);
			mVertexFaceAdjacency.insert(std::make_pair(aVertex, aFace));
		}
	}

// Go over each face and add the edges to the vertex to vertex adjacency list.
	for(auto & aFace: mFaces) {
		Vertex * v0 = aFace->GetVertex(0);
		Vertex * v1 = aFace->GetVertex(1);
		Vertex * v2 = aFace->GetVertex(2);

		std::vector<Vertex* > neighbors = GetConnectedVertices(v0);
		if (std::find(neighbors.begin(), neighbors.end(), v1) == neighbors.end()) {
			mEdges.insert(std::make_pair(v0, v1));
			mEdges.insert(std::make_pair(v1, v0));
		}

		neighbors = GetConnectedVertices(v1);
		if (std::find(neighbors.begin(), neighbors.end(), v2) == neighbors.end()) {
			mEdges.insert(std::make_pair(v1, v2));
			mEdges.insert(std::make_pair(v2, v1));
		}

		neighbors = GetConnectedVertices(v2);
		if (std::find(neighbors.begin(), neighbors.end(), v0) == neighbors.end()) {
			mEdges.insert(std::make_pair(v2, v0));
			mEdges.insert(std::make_pair(v0, v2));
		}
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
		n = glm::normalize(n);

		q = { n.x,n.y,n.z,glm::dot(-n,v0) };
		Q += glm::outerProduct(q, q);
	}
	return Q;
}

void ProgMesh::PreparePairs() {

	std::vector<Vertex*> neighbors;
	Pair* newPair;
	Vertex vOptimal;
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
			error = glm::dot(vOptimal.mPos, 
				(mQuadrics[&aVertex] + mQuadrics[aNeighbor]) * vOptimal.mPos);

			std::multimap<float, Pair *>::iterator itr = mPairs.insert(std::make_pair(error, newPair));
			mEdgeToPair.insert(std::make_pair(std::make_pair(&aVertex, aNeighbor), itr));
		}
	}
}

void ProgMesh::ConnectedVerticesUpdate(Vertex* updateV, Vertex* vOld, Vertex* vNew) {

	std::vector<Vertex* > neighbors = GetConnectedVertices(updateV);

	for (Vertex* & aVertex : neighbors) {
		if (aVertex == vOld) aVertex = vNew;
	}
}

void ProgMesh::DeletePairsWithNeighbor(Vertex* v) {

	std::vector<Vertex* > neighbors = GetConnectedVertices(v);

	for (Vertex* & aNeighbor : neighbors) {
		auto itr = mEdgeToPair.find(std::make_pair(v, aNeighbor));
		if (itr != mEdgeToPair.end()) {
			mPairs.erase(itr->second);
			mEdgeToPair.erase(itr);
		}
		itr = mEdgeToPair.find(std::make_pair(aNeighbor, v));
		if (itr != mEdgeToPair.end()) {
			mPairs.erase(itr->second);
			mEdgeToPair.erase(itr);
		}
	}
}

// need to update mVector, mFaces, mVertexFaceAdjacency, mEdges, mQuadrics
void ProgMesh::EdgeCollapse(Pair* collapsePair) {
	const Vertex vNewLocal = collapsePair->CalcOptimal();

	Vertex* v0 = collapsePair->v0;
	Vertex* v1 = collapsePair->v1;
	Vertex* vNeighbor;
	bool sharedFace;

	// Insert replacement vertex vNew into master array
	mVertices.push_back(vNewLocal);
	Vertex & vNew = mVertices.back();

	// make adjacency of newV the union of v0 and v1 adjacency lists (w/o duplicates)
	std::vector<Face*> v0Faces = GetAdjacentFaces(v0);
	std::vector<Face*> v1Faces = GetAdjacentFaces(v1);
	std::vector<Face*> alreadyInsertedF;

	for (Face* & aFace : v0Faces) {
    // Make a copy
		Face faceCopy(*aFace);
		faceCopy.ReplaceVertex(v0, &vNew);
		Utilities::ReplaceObject(aFace, faceCopy);

		mVertexFaceAdjacency.insert(std::make_pair(&vNew, aFace));
		alreadyInsertedF.push_back(aFace);

	}
	for (Face* & aFace : v1Faces) {
		sharedFace = false;
		for (Face* & insertedFace : alreadyInsertedF) {
		if (insertedFace == aFace) {					// found a degenerate face

			// remove this face from vertex to face adjacency
			for (unsigned int i = 0; i < 3; i++) {
				if (aFace->GetVertex(i) != v0 && aFace->GetVertex(i) != v1) vNeighbor = aFace->GetVertex(i);
			}
			auto range = mVertexFaceAdjacency.equal_range(vNeighbor);
			for (auto it = range.first; it != range.second; ++it) {
				if (it->second == insertedFace) {
					mVertexFaceAdjacency.erase(it);
				}
			}
			sharedFace = true;

			mFaces.erase(aFace);
			delete aFace; aFace = nullptr; // NOTE: Must call in this order


			}
		}
		if (!sharedFace) {
			mVertexFaceAdjacency.insert(std::make_pair(&vNew, aFace));
			aFace->ReplaceVertex(v1, &vNew);
		}
	}

	// update mEdges
	std::vector<Vertex*> v0Vertex = GetConnectedVertices(v0);
	std::vector<Vertex*> v1Vertex = GetConnectedVertices(v1);

	// remove pairs with soon to be outdated errors
	for (Vertex* & aVertex : v0Vertex) {
		DeletePairsWithNeighbor(aVertex);
	}
	for (Vertex* & aVertex : v1Vertex) {
		DeletePairsWithNeighbor(aVertex);
	}

	std::vector<Vertex*> alreadyInsertedV;
	bool sharedVertex;

	for (Vertex* & aVertex : v0Vertex) {
		if (aVertex == v1) continue;
		mEdges.insert(std::make_pair(&vNew, aVertex));
		alreadyInsertedV.push_back(aVertex);
		ConnectedVerticesUpdate(aVertex, v0, &vNew);
	}
	for (Vertex* & aVertex : v1Vertex) {
		if (aVertex == v0) continue;
		sharedVertex = false;
		for (Vertex* & insertedVertex : alreadyInsertedV) {
			if (insertedVertex == aVertex) {					//found a degenerate edge
				// remove degenerate edge
				auto range = mEdges.equal_range(insertedVertex);
				for (auto it = range.first; it != range.second; ++it) {
					if (it->second == v1) {
						mEdges.erase(it);
					}
				}
				sharedVertex = true;
			}
		}
		if (!sharedVertex) {
			mEdges.insert(std::make_pair(&vNew, aVertex));
			ConnectedVerticesUpdate(aVertex, v1, &vNew);
		}
	}

	// update mQuadrics
	std::vector<Vertex* > surroundingVertices = GetConnectedVertices(&vNew);
	for (Vertex* & aVertex : surroundingVertices) {
		auto itr = mQuadrics.find(aVertex);
		itr->second = ComputeQuadric(aVertex);
	}
	mQuadrics.insert(std::make_pair(&vNew, ComputeQuadric(&vNew)));

	// add pairs (now that quadrics are updated need to reorder tree for new errors)
	std::vector<Vertex*> neighbors;
	Pair* newPair;
	Vertex vOptimal;
	float error;

	for (Vertex* & aVertex : surroundingVertices) {
		neighbors = GetConnectedVertices(aVertex);
		for (Vertex* & aNeighbor : neighbors) {
			newPair = new Pair(aVertex, aNeighbor);

			vOptimal = newPair->CalcOptimal();
			error = glm::dot(vOptimal.mPos,
				(mQuadrics[aVertex] + mQuadrics[aNeighbor]) * vOptimal.mPos);

			std::multimap<float, Pair *>::iterator itr = mPairs.insert(std::make_pair(error, newPair));
			mEdgeToPair.insert(std::make_pair(std::make_pair(aVertex, aNeighbor), itr));
		}
	}

	// delete v0 and v1 and update mVertices
	mVertices.erase(
		std::remove_if(mVertices.begin(), mVertices.end(), [v0] (Vertex & v) {
			return v == (*v0);
		}),
		mVertices.end()
		);
	mVertices.erase(
		std::remove_if(mVertices.begin(), mVertices.end(), [v1] (Vertex & v) {
			return v == (*v1);
		}),
		mVertices.end()
		);
	// Insert replacement vertex vNew into master array
	// mVertices.emplace_back((*vNew));
}

void ProgMesh::CollapseLeastError() {

	auto itr = mPairs.begin();
	EdgeCollapse(itr->second);
}

// just used for testing specific collapses
// in practice will only use min error pair, never have to search for pair given v0 v1 except here
void ProgMesh::TestEdgeCollapse(unsigned int v0, unsigned int v1) {
	Vertex* vStart = &mVertices[v0];
	Vertex* vEnd = &mVertices[v1];
	glm::vec4 vOptimal = (vStart->mPos + vEnd->mPos / 2.0f);

	float error = glm::dot(vOptimal,(mQuadrics[vStart] + mQuadrics[vEnd]) * vOptimal);
	auto itr = mPairs.find(error);

	if (itr == mPairs.end()) {
		std::cerr << "Pair not found!" << std::endl;
		return;
	}

	EdgeCollapse(itr->second);
}

void ProgMesh::GenerateIndicesFromFaces() {
	mIndices.clear();
	mIndices.reserve(mFaces.size() * 3);
    // Order doesn't matter for indices.
	for(auto faceItr = mFaces.begin(); faceItr != mFaces.end(); faceItr++) {
		mIndices.push_back(std::find(mVertices.begin(), mVertices.end(), *((*faceItr)->GetVertex(0))) - mVertices.begin());
		mIndices.push_back(std::find(mVertices.begin(), mVertices.end(), *((*faceItr)->GetVertex(1))) - mVertices.begin());
		mIndices.push_back(std::find(mVertices.begin(), mVertices.end(), *((*faceItr)->GetVertex(2))) - mVertices.begin());
	}
}
