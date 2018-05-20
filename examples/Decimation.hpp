#pragma once
#include <unordered_set>

#include "Geometry.hpp"

/// Represents a decimation operation on a ProgMesh. Stores all information regarding that operation
/// so that it can be reversed.
class Decimation {
public:
    Decimation() = default;
    ~Decimation() = default;
    
    Decimation(const Decimation & d) :
    v0(d.v0), v1(d.v1), vNew(d.vNew), v0Faces(d.v0Faces),
    v1Faces(d.v1Faces), degenFaces(d.degenFaces),
    v0Neighbors(d.v0Neighbors), v1Neighbors(d.v1Neighbors)
    {}
    
    Vertex * v0 = nullptr, * v1 = nullptr;
    Vertex * vNew = nullptr;
    
    /// The faces that v0 was a part of, not including faces that both v0 and v1 were part of
    std::vector<Face *> v0Faces;
    /// The faces that v1 was a part of, not including faces that both v0 and v1 were part of
    std::vector<Face *> v1Faces;
    /// Faces that v0 and  v1 were both members of that became degeneratea due to the collapse.
    /// NOTE: v0Faces and v1Faces still exist in the mesh, but degen faces are removed from the mesh
    std::vector<Face *> degenFaces;
    
    /// Stores all neighbors of v0, except v1
    std::vector<Vertex *> v0Neighbors;
    /// Stores all neighbors of v1, except v0
    std::vector<Vertex *> v1Neighbors;
    
};
