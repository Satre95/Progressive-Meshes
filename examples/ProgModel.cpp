#include "ProgModel.hpp"

#include <iostream>

ProgModel::ProgModel(const std::string & path) {
    LoadProgModel(path);
}

void ProgModel::LoadProgModel(const std::string &path) {
	// read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, 
		aiProcess_Triangulate 
		| aiProcess_OptimizeMeshes
		| aiProcess_OptimizeGraph
		| aiProcess_ImproveCacheLocality);

        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
        	std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        	return;
        }
        // retrieve the directory path of the filepath
        mDirectory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        ProcessNode(scene->mRootNode, scene);
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

ProgMesh ProgModel::ProcessMesh(aiMesh *mesh, const aiScene *scene) {
	// data to fill
        std::vector<Vertex> vertices;
        vertices.reserve(mesh->mNumVertices);
        std::vector<Face> indices;
        indices.reserve(mesh->mNumFaces);

        // Walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
            glm::vec4 pos;
            glm::vec4 normal(0.f);

            // positions
            pos.x = mesh->mVertices[i].x;
            pos.y = mesh->mVertices[i].y;
            pos.z = mesh->mVertices[i].z;
            pos.w = 1.f;

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
            indices.emplace_back(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
        }
        
        
        // return a mesh object created from the extracted mesh data
        return ProgMesh(vertices, indices);
}

void ProgModel::PrintInfo(std::ostream &ostream) {
    ostream << "Model loaded from directory: " << mDirectory << std::endl;
    ostream << "Model contains " << mMeshes.size()
            << (mMeshes.size() == 1 ? " mesh" : " meshes" ) << std::endl;
    for (size_t i = 0; i < mMeshes.size(); ++i) {
        ostream << '\t';
        ostream << "Mesh " << i << " contains " << mMeshes.at(i).mVertices.size()
                << " vertices and " << mMeshes.at(i).mFaces.size() << " faces." << std::endl;
    }
}