#include "ProgMesh.hpp"
#include "Utilities.hpp"
#include <utility>
#include <algorithm>

size_t Vertex::sCount = 0;
size_t Face::sCount = 0;
bool ProgMesh::sPrintStatements = false;

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

ProgMesh::ProgMesh(): mOpInProgress(false) {}

ProgMesh::ProgMesh(std::vector<Vertex> & _verts, std::vector<uint32_t > & _indices) :
mIndices(_indices),
mOpInProgress(false) {
    mVertices.reserve(_verts.size());
    for (Vertex & aVert : _verts) {
        Vertex * newVert = new Vertex(aVert);
        mVertices.push_back(newVert);
    }
	for (int i = 0; i < _indices.size(); i+=3) {
		mFaces.insert(new Face(mVertices.at(_indices.at(i)), mVertices.at(_indices.at(i+1)), mVertices.at(_indices.at(i+2))));
	}
}

ProgMesh::ProgMesh(const ProgMesh & other): mOpInProgress(false) {
    //TODO: Implement proper copy constructor.
}

ProgMesh::~ProgMesh() {
	for(auto & facePtr: mFaces) {
		delete facePtr;
	}
    
    for (auto & vertPtr : mVertices) {
        delete vertPtr;
    }
}

void ProgMesh::AllocateBuffers(starforge::RenderDevice &renderDevice) {
    if(mVAO) renderDevice.DestroyVertexArray(mVAO);
    if(mVBO) renderDevice.DestroyVertexBuffer(mVBO);
    if(mIBO) renderDevice.DestroyIndexBuffer(mIBO);
    if(mVertexDescription) renderDevice.DestroyVertexDescription(mVertexDescription);
    
    // Make a local contiguous array to copy verts into GPU buffer
    std::vector<Vertex> localVerts;
    localVerts.reserve(mVertices.size());
    for (auto & vertPtr : mVertices) {
        localVerts.push_back(*vertPtr);
    }

    mVBO = renderDevice.CreateVertexBuffer(mVertices.size() * sizeof(Vertex), localVerts.data());
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

void ProgMesh::PreparePairsAndQuadrics() {
	for (Vertex *& aVertex : mVertices) {
        // Compute quadric for each vertex
		mQuadrics.insert(std::make_pair(aVertex, ComputeQuadric(aVertex)));

        // Compute error for each pair and order them
		auto neighbors = GetConnectedVertices(aVertex);
		for (Vertex* & aNeighbor : neighbors) {
			Pair newPair(aVertex, aNeighbor);

            // only midpoint TODO - can definetly make this the legit optimal w/o too much trouble
			Vertex vOptimal = newPair.CalcOptimal();
            float error = glm::dot(vOptimal.mPos,
				(mQuadrics[aVertex] + mQuadrics[aNeighbor]) * vOptimal.mPos);

			auto itr = mPairs.insert(std::make_pair(error, newPair));
			mEdgeToPair.insert(std::make_pair(std::make_pair(aVertex, aNeighbor), itr));
		}
	}
}

void ProgMesh::PreparePairs() {
	for (Vertex *& aVertex : mVertices) {

		// Compute error for each pair and order them
		auto neighbors = GetConnectedVertices(aVertex);
		for (Vertex* & aNeighbor : neighbors) {
			Pair newPair(aVertex, aNeighbor);

			// only midpoint TODO - can definetly make this the legit optimal w/o too much trouble
			Vertex vOptimal = newPair.CalcOptimal();
			float error = glm::dot(vOptimal.mPos,
				(mQuadrics[aVertex] + mQuadrics[aNeighbor]) * vOptimal.mPos);

			auto itr = mPairs.insert(std::make_pair(error, newPair));
			mEdgeToPair.insert(std::make_pair(std::make_pair(aVertex, aNeighbor), itr));
		}
	}
}


void ProgMesh::DeletePairsWithNeighbor(Vertex* v, std::vector<Vertex* > neighbors, Decimation & dec) {

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

void ProgMesh::CalculateAndStorePair(Vertex* vA, Vertex * vB) {

	Pair pairAB(vA, vB);
	Pair pairBA(vB, vA);

	Vertex vOptimal = pairAB.CalcOptimal();
	float error = glm::dot(vOptimal.mPos,
			(mQuadrics[vA] + mQuadrics[vB]) * vOptimal.mPos);


	auto itr = mPairs.insert(std::make_pair(error, pairAB));
	mEdgeToPair.insert(std::make_pair(std::make_pair(vA, vB), itr));

	itr = mPairs.insert(std::make_pair(error, pairBA));
	mEdgeToPair.insert(std::make_pair(std::make_pair(vB, vA), itr));
	
}

// need to update mVector, mFaces, mVertexFaceAdjacency, mEdges, mQuadrics
void ProgMesh::EdgeCollapse(Pair* collapsePair) {
    Vertex * vNew = new Vertex(collapsePair->CalcOptimal());
    Vertex* v0 = collapsePair->v0;
    Vertex* v1 = collapsePair->v1;
    Decimation decimation;
    // Save v0, v1, and vNew in decimation object
    decimation.vNew = vNew;
    decimation.v0 = v0;
    decimation.v1 = v1;
    
    
    // Insert replacement vertex vNew into master array
    mVertices.push_back(vNew);
    
    
    // 1. Update Faces ( Create new faces, remove degenerates)
    UpdateFaces(v0, v1, *vNew, decimation);
    
    // 2. Update Edges (Create new edges, delete degenerates)
	std::vector<Vertex* > neighbors = UpdateEdgesAndQuadrics(v0, v1, *vNew, decimation);

    // 3. Remove v0 and v1 from master vertices array.
    // TODO: Replace deletion with move to decimation object
    mVertices.erase(std::remove_if(mVertices.begin(), mVertices.end(), [v0, v1] (Vertex *& v) {
        return (*v == *v0) || (*v == *v1);
    }), mVertices.end());

	// 4. Update Pairs
	UpdatePairs(v0, v1, *vNew, neighbors, decimation);

	// 5. (Regen indices for rendering)
	GenerateIndicesFromFaces();
    
    // 6. Add decimation to list
    mDecimations.push(decimation);

}

bool ProgMesh::Downscale() {
	if (mPairs.empty() || mOpInProgress) return false;
    
    mOpInProgress = true;
    
    // First check if we had previously schedule a collapse.
    if(mScheduledCollapse != nullptr) {
        if (sPrintStatements) std::cout << "Collapsing pair: " << mScheduledCollapse->v0 << ", " << mScheduledCollapse->v1 << std::endl;
        EdgeCollapse(mScheduledCollapse);
        if (sPrintStatements) PrintConnectivity(std::cout);
        mScheduledCollapse = nullptr;
        mOpInProgress = false;
    }
    // If not, start the animation and schdule it for later.
    else {
        mScheduledCollapse =  &(mPairs.begin()->second);
        Vertex* v0 = mScheduledCollapse->v0;
        Vertex* v1 = mScheduledCollapse->v1;
        
        glm::vec3 start0(v0->mPos);
        glm::vec3 start1(v1->mPos);
        glm::vec3 end = mScheduledCollapse->CalcOptimal().mPos;
        mVerticesInMotion.insert(std::make_pair(v0, std::make_pair(start0, end)));
        mVerticesInMotion.insert(std::make_pair(v1, std::make_pair(start1, end)));
        mVertexTime.insert(std::make_pair(v0, 0.0));
        mVertexTime.insert(std::make_pair(v1, 0.0));
    }
	return true;
}

// just used for testing specific collapses
// in practice will only use min error pair, never have to search for pair given v0 v1 except here
void ProgMesh::TestEdgeCollapse(unsigned int v0, unsigned int v1) {
	Vertex* vStart = mVertices.at(v0);
	Vertex* vEnd = mVertices.at(v1);
	glm::vec4 vOptimal = (vStart->mPos + vEnd->mPos / 2.0f);

	float error = glm::dot(vOptimal,(mQuadrics[vStart] + mQuadrics[vEnd]) * vOptimal);
	auto itr = mPairs.find(error);

	if (itr == mPairs.end()) {
		std::cerr << "Pair not found!" << std::endl;
		return;
	}

	EdgeCollapse(&(itr->second));
}

void ProgMesh::GenerateIndicesFromFaces() {
	mIndices.clear();
	mIndices.reserve(mFaces.size() * 3);
    // Order doesn't matter for indices.
	for(auto faceItr = mFaces.begin(); faceItr != mFaces.end(); faceItr++) {
		mIndices.push_back(std::find(mVertices.begin(), mVertices.end(), (*faceItr)->GetVertex(0)) - mVertices.begin());
		mIndices.push_back(std::find(mVertices.begin(), mVertices.end(), (*faceItr)->GetVertex(1)) - mVertices.begin());
		mIndices.push_back(std::find(mVertices.begin(), mVertices.end(), (*faceItr)->GetVertex(2)) - mVertices.begin());
	}
}

void ProgMesh::GenerateNormals() {
#pragma omp parallel for
    for (int i = 0; i < mVertices.size(); i++) {
        Vertex *& vert = mVertices.at(i);
        auto adjFaces = GetAdjacentFaces(vert);
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
            vert->mNormal += (adjFaces.at(j)->GetNormal() * faceAreas.at(j));
        }
    }
    
    // Re-normalize all normals
#pragma omp parallel for
    for (int i = 0; i < mVertices.size(); i++) {
        mVertices.at(i)->mNormal = glm::normalize(mVertices.at(i)->mNormal);
    }
}

void ProgMesh::UpdateFaces(Vertex * v0, Vertex * v1, Vertex & vNew, Decimation & dec) {
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
    
    // Keep track of faces in decimation object
    dec.v0Faces = v0Faces;
    dec.v1Faces = v1Faces;
    dec.degenFaces = degenFaces;
    
    // Now, for each degenerate face, remove it from its vertex to face adjacencies
    // In doing so, each degen face is removed for v0 and v1 in master arrays
    for(auto *& aDegenFace: degenFaces) {
        for (size_t i = 0; i < 3; i++) {
            auto range = mVertexFaceAdjacency.equal_range(aDegenFace->GetVertex(i));
            for (auto it = range.first; it != range.second;) {
                if (*(it->second) == (*aDegenFace)) {
                    it = mVertexFaceAdjacency.erase(it);
                } else it++;
            }
        }
    }
    
    // Now, delete degen faces from master faces list
    for(auto *& aDegenFace: degenFaces) {
        mFaces.erase(aDegenFace);
        aDegenFace = nullptr;
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
    // The decimation object is now storing the degenerate faces that were removed and other faces that were modified.
}

std::vector<Vertex* > ProgMesh::UpdateEdgesAndQuadrics(Vertex * v0, Vertex * v1, Vertex & newVertex, Decimation & dec) {
    auto v0Neighbors = GetConnectedVertices(v0);
    auto v1Neighbors = GetConnectedVertices(v1);
    std::sort(v0Neighbors.begin(), v0Neighbors.end());
    std::sort(v1Neighbors.begin(), v1Neighbors.end());
    // Make a union, so there is just one array we have to deal with
    std::vector<Vertex*> allNeighbors;
    allNeighbors.reserve(v0Neighbors.size() + v1Neighbors.size() - 2);
    std::set_union(v0Neighbors.begin(), v0Neighbors.end(), v1Neighbors.begin(), v1Neighbors.end(), std::back_inserter(allNeighbors));
    
    /// Fance-shmancy way to remove v0 and v1 from v1Neighbors and v0Neighbors respectfully
    std::vector<Vertex *> decV0Neighbors; std::vector<Vertex*> v0Vec; v0Vec.push_back(v0);
    std::vector<Vertex *> decV1Neighbors; std::vector<Vertex*> v1Vec; v1Vec.push_back(v1);
    std::set_difference(v0Neighbors.begin(), v0Neighbors.end(), v1Vec.begin(), v1Vec.end(), std::back_inserter(decV0Neighbors));
    std::set_difference(v1Neighbors.begin(), v1Neighbors.end(), v0Vec.begin(), v0Vec.end(), std::back_inserter(decV1Neighbors));
    dec.v0Neighbors = decV0Neighbors; // We bougie up in this bitch
    dec.v1Neighbors = decV1Neighbors;
    
    // 'Edges' are really directed edges between two vertices
    // Remove all the incoming edges into v0 & v1 from their neighbors.
    for(auto *& aNeighbor: allNeighbors) {
		//pairs
        auto range = mEdges.equal_range(aNeighbor);
        for(auto it = range.first; it != range.second;) {
            if (it->second == v0) {
                it = mEdges.erase(it);
            } else it++;
        }
    }
    for(auto *& aNeighbor: allNeighbors) {
        auto range = mEdges.equal_range(aNeighbor);
        for(auto it = range.first; it != range.second;) {
            if (it->second == v1) {
                it = mEdges.erase(it);
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

	// update mQuadrics for vertices whose adjacent planes have changed
	for (auto & aNeighbor : allNeighbors) {
		auto itr = mQuadrics.find(aNeighbor);
		itr->second = ComputeQuadric(aNeighbor);
	}
	// add a quadric for the new vertex
	mQuadrics.insert(std::make_pair(&newVertex, ComputeQuadric(&newVertex)));

	return allNeighbors;
}

void ProgMesh::UpdatePairs(Vertex * v0, Vertex * v1, Vertex & newVertex, std::vector<Vertex* > neighbors, Decimation & dec)
{
    /*
	// Deleting all pairs with v0 and v1 as one of the vertices
	DeletePairsWithNeighbor(v0, neighbors);
	DeletePairsWithNeighbor(v1, neighbors);

	// Delete the rest of the pairs that are connected to vertices whose quadrics have been updated
	for (auto & aNeighbor : neighbors) {
		auto inLaws = GetConnectedVertices(aNeighbor);
		DeletePairsWithNeighbor(aNeighbor, inLaws);
	}

	// add pairs from every vertex whose quadric was affected to its new neighbors
	// (since edges have already been updated this includes the new vertex)
	for (auto & aNeighbor : neighbors) {
		auto inLaws = GetConnectedVertices(aNeighbor);
		for (auto & inLaw : inLaws) {
			CalculateAndStorePair(aNeighbor, inLaw);
		}
	}

	// delete pairs between v0 and v1
	auto itr = mEdgeToPair.find(std::make_pair(v0, v1));
	if (itr != mEdgeToPair.end()) {
		auto numErased = mPairs.erase(itr->second->first);
		mEdgeToPair.erase(itr);
    }
    
	itr = mEdgeToPair.find(std::make_pair(v1, v0));
	if (itr != mEdgeToPair.end()) {
		auto numErased = mPairs.erase(itr->second->first);
		mEdgeToPair.erase(itr);
    }
    */
    


    // Clear all pairs and regen.
    //TODO: Fix the individual updating above
    mPairs.clear();
    mEdgeToPair.clear();

    PreparePairs();
//    for (auto & aPair : mPairs) {
//        if( (*aPair.second.v0 == *v0 || *aPair.second.v1 == *v0) && (*aPair.second.v0 == *v1 || *aPair.second.v1 == *v1)) {
//            std::cout << "ERROR: Found v0 and v1 pair in mPairs after it should have been removed" << std::endl;
//        }
//    }
}

/// After all operations for a particular edge collapse have been performed, need to update the GPU buffers
void ProgMesh::UpdateBuffers(starforge::RenderDevice & renderDevice) {
    // Make a local contiguous array to copy verts into GPU buffer
    std::vector<Vertex> localVerts;
    localVerts.reserve(mVertices.size());
    for (auto & vertPtr : mVertices) {
        localVerts.push_back(*vertPtr);
    }
    
    renderDevice.FillVertexBuffer(mVBO, localVerts.size() * sizeof(localVerts.front()), localVerts.data());
    renderDevice.FillIndexBuffer(mIBO, mIndices.size() * sizeof(mIndices.front()), mIndices.data());
}

bool ProgMesh::Upscale() {
    if (mDecimations.empty() || mOpInProgress) return false;
    // For Upscale, perform the operation first, then do the animation
    mOpInProgress = true;
    
    // Get most recently inserted decimation
    Decimation decimation = mDecimations.top(); mDecimations.pop();
    
    // 1. Create and reinsert faces into ajacencey list
    RecreateFaces(decimation);
    
    // 2. Create and reinsert edges
    RecreateEdgesAndQuadrics(decimation);
    
    // 3. Create and update pairs
    RecreatePairs(decimation);
    
    // 4. Delete vNew from master vertex list
    Vertex* vNew = decimation.vNew;
    mVertices.erase(std::remove_if(mVertices.begin(), mVertices.end(), [vNew](Vertex *& v) {
        return (*v == *vNew);
    }), mVertices.end());
    glm::vec3 startPos = decimation.vNew->mPos;
    delete decimation.vNew;
    
    // 5. Reinsert v0 and v1 into the master vertex list
    mVertices.push_back(decimation.v0);
    mVertices.push_back(decimation.v1);
    
    // 6. Setup and schedule the animation
    auto end_v0 = glm::vec3(decimation.v0->mPos);
    auto end_v1 = glm::vec3(decimation.v1->mPos);
    mVerticesInMotion.insert(std::make_pair(decimation.v0, std::make_pair(startPos, end_v0)));
    mVerticesInMotion.insert(std::make_pair(decimation.v1, std::make_pair(startPos, end_v1)));
    mVertexTime.insert(std::make_pair(decimation.v0, 0.0));
    mVertexTime.insert(std::make_pair(decimation.v1, 0.0));
    decimation.v0->mPos = glm::vec4(startPos, 1.f);
    decimation.v1->mPos = glm::vec4(startPos, 1.f);
    
    // 7. Regenerate indicies for update
    GenerateIndicesFromFaces();
    
	return true;

}

void ProgMesh::RecreateFaces(Decimation & decimation) {

	Vertex * v0 = decimation.v0;
	Vertex * v1 = decimation.v1;
	Vertex * vNew = decimation.vNew;

	auto & v0Faces = decimation.v0Faces;
	auto & v1Faces = decimation.v1Faces;

	// Replacing all face indicies with vNew in them to have v0 or v1
	for (auto aFacePtr : v0Faces) {
		aFacePtr->ReplaceVertex(vNew, v0);
		mVertexFaceAdjacency.insert(std::make_pair(v0, aFacePtr));
	}
	for (auto aFacePtr : v1Faces) {
		aFacePtr->ReplaceVertex(vNew, v1);
		mVertexFaceAdjacency.insert(std::make_pair(v1, aFacePtr));
	}

	// Re-add the degenerate faces and update vertex to face adjacency for v0, v1, and the vertex neighbors shared by them
	for (auto aDegenPtr : decimation.degenFaces) {
		mFaces.insert(aDegenPtr);
		mVertexFaceAdjacency.insert(std::make_pair(aDegenPtr->GetVertex(0), aDegenPtr));
		mVertexFaceAdjacency.insert(std::make_pair(aDegenPtr->GetVertex(1), aDegenPtr));
		mVertexFaceAdjacency.insert(std::make_pair(aDegenPtr->GetVertex(2), aDegenPtr));
	}

}

void ProgMesh::RecreateEdgesAndQuadrics(Decimation & decimation) {

	Vertex * v0 = decimation.v0;
	Vertex * v1 = decimation.v1;
	Vertex * vNew = decimation.vNew;

	// Reinsert removed edges between v0, v1 and their respective neighbors
	for (auto & aNeighbor : decimation.v0Neighbors) {
		mEdges.insert(std::make_pair(v0, aNeighbor));
		mEdges.insert(std::make_pair(aNeighbor, v0));
	}
	for (auto & aNeighbor : decimation.v1Neighbors) {
		mEdges.insert(std::make_pair(v1, aNeighbor));
		mEdges.insert(std::make_pair(aNeighbor, v1));
	}
	mEdges.insert(std::make_pair(v0, v1));
	mEdges.insert(std::make_pair(v1, v0));

	// Getting the neighbors of vNew (w/o duplication)
	std::vector<Vertex* > vNewNeighbors;
	vNewNeighbors.reserve(decimation.v0Neighbors.size() + decimation.v1Neighbors.size() - decimation.degenFaces.size());
	std::set_union(decimation.v0Neighbors.begin(), decimation.v0Neighbors.end(), decimation.v1Neighbors.begin(), 
												decimation.v1Neighbors.end(), std::back_inserter(vNewNeighbors));

	// Delete outbound edges of vNew
	mEdges.erase(vNew);

	// Delete the inbound edges of vNew
	for (auto & aNeighbor : vNewNeighbors) {
		auto range = mEdges.equal_range(aNeighbor);
		for (auto it = range.first; it != range.second;) {
			if (it->second == vNew) {
				it = mEdges.erase(it);
			}
			else it++;
		}
	}

	// Update mQuadrics for vertices whose adjacent faces have changed
	for (auto & aNeighbor : vNewNeighbors) {
		auto itr = mQuadrics.find(aNeighbor);
		itr->second = ComputeQuadric(aNeighbor);
	}
	auto itr = mQuadrics.find(v0);
	itr->second = ComputeQuadric(v0);
	itr = mQuadrics.find(v1);
	itr->second = ComputeQuadric(v1);
	mQuadrics.erase(vNew);

}

void ProgMesh::RecreatePairs(Decimation & decimation) {
	mPairs.clear();
	mEdgeToPair.clear();
	PreparePairs();
}

void ProgMesh::Animate(double delta_t, starforge::RenderDevice & renderDevice) {
    // Update the time values for each vertex in motion
    for(auto & aVertexPath: mVerticesInMotion) {
        Vertex * vertex = aVertexPath.first;
        
        double time = mVertexTime.at(vertex);
//        time = glm::clamp(time + delta_t, 0.0, 1.0);
        time = glm::clamp(time + 0.1, 0.0, 1.0);
        mVertexTime.at(vertex) = time;
    }
    
    // Perform the interpolation translation
    auto vMItr = mVerticesInMotion.begin();
    while(vMItr != mVerticesInMotion.end()) {
        Vertex * vertex = vMItr->first;
        auto & startPos = vMItr->second.first;
        auto & endPos = vMItr->second.second;
        
        double time = mVertexTime.at(vertex);
        auto newPos = glm::mix(startPos, endPos, time);
        vertex->mPos = glm::vec4(newPos, 1.f);
        
        vMItr++;
    }
    
    UpdateBuffers(renderDevice);
    
    CheckAnimations();
}

void ProgMesh::CheckAnimations() {
    for (auto & aVertexPtr : mVertices) {
        auto searchItr = mVertexTime.find(aVertexPtr);
        if(searchItr != mVertexTime.end()) {
            if (searchItr->second >= 1.0) {
                mVertexTime.erase(aVertexPtr);
                mVerticesInMotion.erase(aVertexPtr);
            }
        }
    }
    mOpInProgress = !(mVerticesInMotion.empty());
}

