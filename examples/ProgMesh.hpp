#pragma once

#include <glm/matrix.hpp>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <functional>
#include <vector>
#include <iostream>
#include <stack>
#include <memory>
#include <atomic>
#include "Geometry.hpp"
#include "RenderDevice.hpp"
#include "Decimation.hpp"

/**
 * This class represents geometry in space and any associated transformations on that geometry.
 */
class ProgMesh
{
public:
	ProgMesh();
	//ProgMesh(std::vector<Vertex> & _verts, std::unordered_set<Face> & _faces);
    ProgMesh(std::vector<Vertex> & _verts, std::vector<uint32_t > & _indices);
    ProgMesh(const ProgMesh & other);
    ~ProgMesh();

	const glm::mat4 & GetModelMatrix() const { return  mModelMatrix; }
	glm::mat4 & GetModelMatrix() { return mModelMatrix; }

	void AllocateBuffers(starforge::RenderDevice & renderDevice);
    void Draw(starforge::RenderDevice & renderDevice);
    void BuildConnectivity();
    void PrintConnectivity(std::ostream & os);
    /// Returns a list of vertices that are connected vertices, i.e. vertices that have an edge to the given vertex
    std::vector<Vertex *> GetConnectedVertices(Vertex *) const;
    /// Returns a list of faces that the given vertex is a part of.
    std::vector<Face *> GetAdjacentFaces(Vertex *) const;
	glm::mat4 ComputeQuadric(Vertex * aVertex) const;
	void EdgeCollapse(Pair* collapsePair);
	void TestEdgeCollapse(unsigned int v0, unsigned int v1);
	bool Downscale(size_t numOps = 50);
	bool Upscale(size_t numOps = 50);
	void GenerateNormals();
    
    void UpdateBuffers(starforge::RenderDevice & renderDevice);

    void Animate(double delta_t);
	static bool sPrintStatements;
private:

    /// Computes initial quadrics and pairs and sorts the latter by smallest error
    void PreparePairsAndQuadrics();
	void PreparePairs();
	void GenerateIndicesFromFaces();
	void DeletePairsWithNeighbor(Vertex* v, std::vector<Vertex* > neighbors, Decimation & dec);
	void CalculateAndStorePair(Vertex* vA, Vertex * vB);
    void UpdateFaces(Vertex * v0, Vertex * v1, Vertex & newVertex, Decimation & dec);
	std::vector<Vertex* > UpdateEdgesAndQuadrics(Vertex * v0, Vertex * v1, Vertex & newVertex, Decimation & dec);
	void UpdatePairs(Vertex * v0, Vertex * v1, Vertex & newVertex, std::vector<Vertex* > neighbors, Decimation & dec);
    
	void RecreateFaces(Decimation & decimation);
	void RecreateEdgesAndQuadrics(Decimation & decimation);
	void RecreatePairs(Decimation & decimation);
    
    /// Called by Animate() removes animation that are completed
    void CheckAnimations();

    /// The list of decimation operations that have occurred
    std::stack<Decimation> mDecimations;
    
	/// The vertices that compose this ProgMesh
	std::vector<Vertex *> mVertices;

	/// Stores the faces. Faces are indices into the vertex array.
	//std::vector<Face> mFaces;
    std::unordered_set<Face *, FacePtrHash> mFaces;
	std::vector<uint32_t> mIndices;

	/// The vertex to face adjacency.
	std::unordered_multimap<Vertex*, Face*, VertexPtrHash> mVertexFaceAdjacency;

	/// The vertex adjacency (i.e. edges)
	std::unordered_multimap<Vertex *, Vertex *, VertexPtrHash> mEdges;

	/// The vertex quadrics
	std::unordered_map<Vertex *, glm::mat4, VertexPtrHash> mQuadrics;

	/// The pairs ordered by error
	std::multimap<float,Pair> mPairs;

	// map from two vertices to iterator that points into mPairs
	// this allows access and updating of mPairs, given the two verticies that make up the pair
	std::unordered_map<std::pair<Vertex*, Vertex*>, std::multimap<float, Pair>::iterator> mEdgeToPair;
    
    /// Tracks vertices that are currently being moved for geomorphing animation
    /// Stores the start and end positions of the vertices
    std::unordered_map<Vertex *, std::pair<glm::vec3, glm::vec3>> mVerticesInMotion;

    /// Associates each vertex in motion with it's normalized time value
    std::unordered_map<Vertex *, double> mVertexTime;
    
    /// Flag the signifies whether an operation (including animation) is in progress.
    std::atomic_bool mOpInProgress;
    
	glm::mat4 mModelMatrix;

	/// The GPU buffers are managed by the render device.
	/// If you want to delete them manually, you must do it through one of the
	/// associated Destroy____Buffer methods.
	/// DO NOT call delete buf at any time.
	starforge::VertexArray * mVAO = nullptr;
	starforge::VertexBuffer * mVBO = nullptr;
	starforge::IndexBuffer * mIBO = nullptr;
    starforge::VertexDescription * mVertexDescription = nullptr;
	friend class ProgModel;
};

typedef std::shared_ptr<ProgMesh> ProgMeshRef;
