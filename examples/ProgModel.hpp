#pragma once
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "ProgMesh.hpp"
#include <iostream>

/**
 * This class represents a model as exists and is loaded from a file. Actual geometry is stored in one or more meshes
 * that are create from the loaded data.
*/
class ProgModel
{
public:
	ProgModel(std::string const &path);

	void LoadProgModel(std::string const & path);
	void PrintInfo(std::ostream & ostream);

	const std::vector<ProgMesh> & GetMeshes() const { return mMeshes; }
	std::vector<ProgMesh> & GetMeshes() { return mMeshes; }
private:

	void ProcessNode(aiNode *node, const aiScene *scene);
	ProgMesh ProcessMesh(aiMesh *mesh, const aiScene *scene);

	std::vector<ProgMesh> mMeshes;

	std::string mDirectory;	
};