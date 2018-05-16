#include "ProgModel.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

ProgModel::ProgModel(const std::string & path) {
//     LoadProgModel(path);
    LoadOFF(path);
}


void ProgModel::LoadOFF(std::string const & path) {
    std::ifstream file(path);
    if(!file.good()) {
        std::cerr << "ERROR: File " << path << " does not exist." << std::endl;  
    }

    //Read first line to make sure it is OFF
    std::string offLine;
    std::getline(file, offLine);
    if(offLine != "OFF") {
        std::cerr << "ERROR: File is not and OFF file" << std::endl;
    }

    // Read number of vertices and faces
    std::string numVertsAndFaces;
    std::getline(file, numVertsAndFaces);
    std::stringstream numBuf(numVertsAndFaces);
    size_t numVerts, numFaces;
    numBuf >> numVerts >> numFaces;

    std::vector<Vertex> vertices;
    for (int i = 0; i < numVerts; ++i) {
        std::string vertLine;
        std::getline(file, vertLine);
        std::stringstream components(vertLine);
        float x, y, z;
        components >> x >> y >> z;
        vertices.emplace_back(glm::vec4(x, y, z, 1.0f));
    }  

    std::vector<unsigned int> indices;
    for (int i = 0; i < numFaces; ++i)
    {
        std::string indicesLine;
        std::getline(file, indicesLine);
        std::stringstream indicesBuf(indicesLine);
        size_t num, i0, i1, i2;
        indicesBuf >> num >> i0 >> i1 >> i2;
        indices.push_back(i0); indices.push_back(i1); indices.push_back(i2);
    }

    ProgMeshRef mesh = std::make_shared<ProgMesh>(vertices, indices);
    mMeshes.push_back(mesh);

#pragma omp parallel for
    for (int i = 0; size_t(i) < mMeshes.size(); ++i) {
        mMeshes.at(i)->BuildConnectivity();
        mMeshes.at(i)->PreparePairs();
        mMeshes.at(i)->GenerateNormals();
    }
}

void ProgModel::LoadProgModel(const std::string &path) {
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path,
                                             //aiProcess_Triangulate
                                             //| aiProcess_OptimizeMeshes
                                             //| aiProcess_OptimizeGraph
                                             //| aiProcess_ImproveCacheLocality
                                              aiProcess_GenNormals
											  | aiProcess_JoinIdenticalVertices);

    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
    // retrieve the directory path of the filepath
    mDirectory = path.substr(0, path.find_last_of('/'));

    // process ASSIMP's root node recursively
    ProcessNode(scene->mRootNode, scene);

    // Once all models are loaded, ask them to build their mesh connectivity data structures.
#pragma omp parallel for
    for (int i = 0; size_t(i) < mMeshes.size(); ++i) {
        mMeshes.at(i)->BuildConnectivity();
		mMeshes.at(i)->PreparePairs();
    }
}

void ProgModel::ProcessNode(aiNode *node, const aiScene *scene) {
// process each mesh located at the current node
	for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		mMeshes.push_back(ProcessMesh(mesh, scene));
	}

    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for(unsigned int i = 0; i < node->mNumChildren; i++) {
		ProcessNode(node->mChildren[i], scene);
	}
}

ProgMeshRef ProgModel::ProcessMesh(aiMesh *mesh, const aiScene *scene) {
	// data to fill
        std::vector<Vertex> vertices;
        vertices.reserve(mesh->mNumVertices);
        std::vector<uint32_t > indices;
        indices.reserve(mesh->mNumFaces * 3);

        // Walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
            glm::vec4 pos(1.f);
            glm::vec4 normal(0.f);

            // positions
            pos.x = mesh->mVertices[i].x;
            pos.y = mesh->mVertices[i].y;
            pos.z = mesh->mVertices[i].z;

            // Normals
            if(mesh->HasNormals()) {
	            // normals
	            normal.x = mesh->mNormals[i].x;
	            normal.y = mesh->mNormals[i].y;
	            normal.z = mesh->mNormals[i].z;
        	}

        	vertices.emplace_back(pos, normal);
        }

        // now wak through each of the mesh's faces and retrieve the corresponding vertex indices.
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace & face = mesh->mFaces[i];
            // Assume we are loading triangle faces.
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }
        
        
        // return a mesh object created from the extracted mesh data
        return std::make_shared<ProgMesh>(vertices, indices);
}

void ProgModel::PrintInfo(std::ostream &ostream) {
    ostream << "Model loaded from directory: " << mDirectory << std::endl;
    ostream << "Model contains " << mMeshes.size()
            << (mMeshes.size() == 1 ? " mesh" : " meshes" ) << std::endl;
    for (size_t i = 0; i < mMeshes.size(); ++i) {
        ostream << '\t';
        ostream << "Mesh " << i << " contains " << mMeshes.at(i)->mVertices.size()
                << " vertices and " << mMeshes.at(i)->mFaces.size() << " faces." << std::endl;
        mMeshes.at(i)->PrintConnectivity(ostream);
    }

}
