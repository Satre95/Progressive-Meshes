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

void ProgMesh::PrintConnectivity(std::ostream & os) {
    //for( Vertex & aVertex: mVertices) {
    //    auto range = mVertexFaceAdjacency.equal_range(&aVertex);
    //    size_t count = 0;
    //    for(auto it = range.first; it != range.second; ++it) {
    //        auto vert = it->first;
    //        auto face = it->second;
    //        count++;
    //    }
    //    os << "\t\tVertex " << aVertex.mId << " is adjacent to " << count << " faces." << std::endl;
    //}

	os << "\t\tThere are " << mEdges.size() / 2.f << " edges in this mesh." << std::endl;
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
    
    // Insert replacement vertex vNew into master array
    mVertices.push_back(vNewLocal);
    Vertex & vNew = mVertices.back();
    
    // 1. Update Faces ( Create new faces, remove degenerates)
    UpdateFaces(v0, v1, vNew);
    
    // 2. Update Edges (Create new edges, delete degenerates)
    UpdateEdgesAndQuadrics(v0, v1, vNew);


	// remove pairs with soon to be outdated errors
//    for (Vertex* & aVertex : v0Vertex) {
//        DeletePairsWithNeighbor(aVertex);
//    }
//    for (Vertex* & aVertex : v1Vertex) {
//        DeletePairsWithNeighbor(aVertex);
//    }


	

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

	GenerateIndicesFromFaces();

}

void ProgMesh::CollapseLeastError() {

	auto itr = mPairs.begin();
	std::cout << "Collapsing pair: " << itr->second->v0 << ", " << itr->second->v1 << std::endl;
	EdgeCollapse(itr->second);
	PrintConnectivity(std::cout);
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

void ProgMesh::GenerateNormals() {
#pragma omp parallel for
    for (int i = 0; i < mVertices.size(); i++) {
        Vertex & vert = mVertices.at(i);
        auto adjFaces = GetAdjacentFaces(&vert);
        std::vector<float> faceAreas;
        faceAreas.reserve(adjFaces.size());
        
        for (int j = 0; j < adjFaces.size(); j++) {
            faceAreas.push_back(adjFaces.at(j)->GetArea());
        }
        // Find the largest area
        float maxArea = *std::max_element(faceAreas.begin(), faceAreas.end());
        // Normalize all areas to 0 ... 1.
        for(float & anArea: faceAreas) anArea /= maxArea;
        
        // Sum normals, weighting by area
        for (int j = 0; j < adjFaces.size(); j++) {
            vert.mNormal += (adjFaces.at(j)->GetNormal() * faceAreas.at(j));
        }
    }
    
    // Re-normalize all normals
#pragma omp parallel for
    for (int i = 0; i < mVertices.size(); i++) {
        mVertices.at(i).mNormal = glm::normalize(mVertices.at(i).mNormal);
    }
}

void ProgMesh::UpdateFaces(Vertex * v0, Vertex * v1, Vertex & vNew) {
    // make adjacency of newV the union of v0 and v1 adjacency lists (w/o duplicates)
    std::vector<Face*> v0Faces = GetAdjacentFaces(v0);
    std::vector<Face*> v1Faces = GetAdjacentFaces(v1);
    
    //Figure out which faces will become degenerate post-collapse.
    std::sort(v0Faces.begin(), v0Faces.end());
    std::sort(v1Faces.begin(), v1Faces.end());
    std::vector<Face*> degenFaces;
    std::set_intersection(v0Faces.begin(), v0Faces.end(), v1Faces.begin(), v1Faces.end(), std::back_inserter(degenFaces));
    
    // Now remove degenerate faces from local v0 and v1 lists
    v0Faces.erase(std::remove_if(v0Faces.begin(), v0Faces.end(), [degenFaces] (Face *& f) {
        for(auto & aDegenFace: degenFaces)
            if (*f == *aDegenFace)
                return true;
        return false;
    }), v0Faces.end());
    v1Faces.erase(std::remove_if(v1Faces.begin(), v1Faces.end(), [degenFaces] (Face *& f) {
        for(auto & aDegenFace: degenFaces)
            if (*f == *aDegenFace)
                return true;
        return false;
    }), v1Faces.end());
    
    // Now, for each degenerate face, remove it from its vertex to face adjacencies
    // In doing so, each degen face is removed for v0 and v1 in master arrays
    for(auto *& aDegenFace: degenFaces) {
        for (size_t i = 0; i < 3; i++) {
            auto range = mVertexFaceAdjacency.equal_range(aDegenFace->GetVertex(i));
            for (auto it = range.first; it != range.second;) {
                if (*(it->second) == (*aDegenFace)) {
                    mVertexFaceAdjacency.erase(it);
                    break;
                } else it++;
            }
        }
    }
    
    // Now, delete degen faces from master faces list
    for(auto *& aDegenFace: degenFaces) {
        mFaces.erase(aDegenFace);
        delete aDegenFace; aDegenFace = nullptr;
    }
    degenFaces.clear(); // Sanity
    
    // Now, iterate over the remainining non-degen faces adj to v0 and v1 and assign new vertex
    for(auto *& v0Face: v0Faces) {
        // Make a copy
        Face faceCopy(*v0Face);
        faceCopy.ReplaceVertex(v0, &vNew);
        Utilities::ReplaceObject(v0Face, faceCopy);
        
        mVertexFaceAdjacency.insert(std::make_pair(&vNew, v0Face));
    }
    for(auto *& v1Face: v1Faces) {
        // Make a copy
        Face faceCopy(*v1Face);
        faceCopy.ReplaceVertex(v1, &vNew);
        Utilities::ReplaceObject(v1Face, faceCopy);
        
        mVertexFaceAdjacency.insert(std::make_pair(&vNew, v1Face));
    }
    
    // At this point, all degenerate faces have been removed, a new vertex has been created,
    // and the mVertexFaceAdjacency has been updated to reflect the removals and creation of new vertex and faces.
}

void ProgMesh::UpdateEdgesAndQuadrics(Vertex * v0, Vertex * v1, Vertex & newVertex) {
    auto v0Neighbors = GetConnectedVertices(v0);
    auto v1Neighbors = GetConnectedVertices(v1);
    std::sort(v0Neighbors.begin(), v0Neighbors.end());
    std::sort(v1Neighbors.begin(), v1Neighbors.end());
    // Make a union, so there is just one array we have to deal with
    std::vector<Vertex*> allNeighbors;
    std::set_union(v0Neighbors.begin(), v0Neighbors.end(), v1Neighbors.begin(), v1Neighbors.end(), std::back_inserter(allNeighbors));
    
    // 'Edges' are really directed edges between two vertices
    // Remove all the incoming edges into v0 v1 from their neighbors.
    for(auto *& aNeighbor: allNeighbors) {
        auto range = mEdges.equal_range(aNeighbor);
        for(auto it = range.first; it != range.second;) {
            if (it->second == v0) {
                mEdges.erase(it);
                break;
            } else it++;
        }
    }
    for(auto *& aNeighbor: allNeighbors) {
        auto range = mEdges.equal_range(aNeighbor);
        for(auto it = range.first; it != range.second;) {
            if (it->second == v1) {
                mEdges.erase(it);
                break;
            } else it++;
        }
    }
    
    // Now remove outgoing edges of v0 and v1
    mEdges.erase(v0);
    mEdges.erase(v1);
    
    // Now create new edges for the vNew
    // First remove v0 and v1 from the allNeighbors array
    allNeighbors.erase(std::remove_if(allNeighbors.begin(), allNeighbors.end(), [v0, v1] (Vertex *& v) {
        return ((*v) == (*v0)) || ((*v) == (*v1));
    }), allNeighbors.end());
    for(auto & aNeighbor: allNeighbors) {
        // Create inbound edge to vNew
        mEdges.insert(std::make_pair(aNeighbor, &newVertex));
        // Create outbound edge from vNew to neighbor
        mEdges.insert(std::make_pair(&newVertex, aNeighbor));
    }
    
    // Done updating edges.

	// remove quadrics for v0 and v1
	mQuadrics.erase(v0);
	mQuadrics.erase(v1);

	// update mQuadrics for vertices whose adjacent planes have changed
	for (auto & aNeighbor : allNeighbors) {
		auto itr = mQuadrics.find(aNeighbor);
		itr->second = ComputeQuadric(aNeighbor);
	}
	// add a quadric for the new vertex
	mQuadrics.insert(std::make_pair(&newVertex, ComputeQuadric(&newVertex)));
    
}

