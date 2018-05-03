#include "Model.hpp"

namespace starforge
{
	Model::Model(const std::string & path, RenderDevice & renderDevice, bool gamma) : m_gammaCorrection(gamma)
	{
		Load(path, renderDevice);
	}

	void Model::Load(std::string path, RenderDevice & renderDevice)
	{
		// read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			return;
		}
		// process ASSIMP's root node recursively
		ProcessNode(scene->mRootNode, scene, renderDevice);

		//Once all meshes have been constructed, tell them to init their buffers.
		for (Mesh * aMesh : m_meshes)
			aMesh->InitBuffers(renderDevice);

		std::cout << "ASSIMP:: Successfully loaded model at " << path << std::endl;
	}

	void Model::ProcessNode(aiNode * node, const aiScene * scene, RenderDevice & renderDevice)
	{
		// process each mesh located at the current node
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			// the node object only contains indices to index the actual objects in the scene.
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_meshes.push_back(ProcessMesh(mesh, scene, renderDevice));
		}
		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene, renderDevice);
		}
	}

	Mesh * Model::ProcessMesh(aiMesh * mesh, const aiScene * scene, RenderDevice & renderDevice)
	{
		// data to fill
		std::vector<float> vertexData;
		std::vector<unsigned int> indices;
		std::vector<Texture2D*> textures;

		//Figure out the size in bytes of the vertex.
		size_t vertexSize = 0;
		if (mesh->HasPositions())
			vertexSize += sizeof(glm::vec3);				//Positions
		// if (mesh->HasNormals())
		// 	vertexSize += sizeof(glm::vec3);				//Normals
		// if (mesh->HasTextureCoords(0))
		// 	vertexSize += sizeof(glm::vec2);				//TexCoords
		// if (mesh->HasTangentsAndBitangents())
		// 	vertexSize += sizeof(glm::vec3) * 2;			//Tangents & Bitangents
		// if (mesh->HasVertexColors(0))
		// 	vertexSize += sizeof(glm::vec4);				//Colors

		std::cout << "Vertex Size in bytes: " << vertexSize << std::endl;


		//Now generate a vertex description.
		//for each vertex attrib, create an element.
		size_t offset = 0;
		std::vector<VertexElement> vertexElements;
		std::cout << "Mesh vertices have ";
		if (mesh->HasPositions())
		{
			std::cout << "positions, ";
			vertexElements.emplace_back(0, VERTEXELEMENTTYPE_FLOAT, 3, vertexSize, offset);
			offset += sizeof(glm::vec3);
		}
		/*
		if (mesh->HasNormals())
		{
			std::cout << "normals, ";
			vertexElements.emplace_back(1, VERTEXELEMENTTYPE_FLOAT, 3, vertexSize, offset);
			offset += sizeof(glm::vec3);
		}
		if (mesh->mTextureCoords[0])
		{
			std::cout << " tex coords, ";
			vertexElements.emplace_back(2, VERTEXELEMENTTYPE_FLOAT, 2, vertexSize, offset);
			offset += sizeof(glm::vec2);
		}
		if (mesh->HasTangentsAndBitangents())
		{
			std::cout << "tangents & bitangents, ";
			vertexElements.emplace_back(3, VERTEXELEMENTTYPE_FLOAT, 3, vertexSize, offset);
			offset += sizeof(glm::vec3);
			vertexElements.emplace_back(4, VERTEXELEMENTTYPE_FLOAT, 3, vertexSize, offset);
			offset += sizeof(glm::vec3);
		}
		if (mesh->HasVertexColors(0))
		{
			std::cout << "vertex colors";
			vertexElements.emplace_back(5, VERTEXELEMENTTYPE_FLOAT, 4, vertexSize, offset);
			offset += sizeof(glm::vec4);
		}
		*/
		std::cout << std::endl;
		//Create the vertex description
		VertexDescription * vertexDescription = renderDevice.CreateVertexDescription((unsigned int)vertexElements.size(), vertexElements.data());

		// Walk through each of the mesh's vertices
		for (unsigned int vCount = 0; vCount < mesh->mNumVertices; vCount++)
		{
			//Order is pos, normal, texCoord, tangent, bitangent, color
			if (mesh->HasPositions())
			{
				glm::vec3 pos;
				pos.x = mesh->mVertices[vCount].x;
				pos.y = mesh->mVertices[vCount].y;
				pos.z = mesh->mVertices[vCount].z;
				for (int i = 0; i < pos.length(); i++)
					vertexData.push_back(pos[i]);
			}
			// if (mesh->HasNormals())
			// {
			// 	glm::vec3 normal;
			// 	// normals
			// 	normal.x = mesh->mNormals[vCount].x;
			// 	normal.y = mesh->mNormals[vCount].y;
			// 	normal.z = mesh->mNormals[vCount].z;
			// 	for (int i = 0; i < normal.length(); i++)
			// 		vertexData.push_back(normal[i]);
			// }
			// // texture coordinates
			// if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			// {
			// 	glm::vec2 texCoord;
			// 	// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
			// 	// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			// 	texCoord.x = mesh->mTextureCoords[0][vCount].x;
			// 	texCoord.y = mesh->mTextureCoords[0][vCount].y;
			// 	for (int i = 0; i < texCoord.length(); i++)
			// 		vertexData.push_back(texCoord[i]);
			// }
			// if (mesh->HasTangentsAndBitangents())
			// {
			// 	// tangent
			// 	glm::vec3 tangent;
			// 	tangent.x = mesh->mTangents[vCount].x;
			// 	tangent.y = mesh->mTangents[vCount].y;
			// 	tangent.z = mesh->mTangents[vCount].z;
			// 	for (int i = 0; i < tangent.length(); i++)
			// 		vertexData.push_back(tangent[i]);
			// 	// bitangent
			// 	glm::vec3 bitangent;
			// 	bitangent.x = mesh->mBitangents[vCount].x;
			// 	bitangent.y = mesh->mBitangents[vCount].y;
			// 	bitangent.z = mesh->mBitangents[vCount].z;
			// 	for (int i = 0; i < bitangent.length(); i++)
			// 		vertexData.push_back(bitangent[i]);
			// }
			// if (mesh->HasVertexColors(0))
			// {
			// 	glm::vec4 color;
			// 	color.r = mesh->mColors[0][vCount].r;
			// 	color.g = mesh->mColors[0][vCount].g;
			// 	color.b = mesh->mColors[0][vCount].b;
			// 	color.a = mesh->mColors[0][vCount].a;
			// 	for (int i = 0; i < color.length(); i++)
			// 		vertexData.push_back(color[i]);
			// }
		}


		// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace & face = mesh->mFaces[i];
			// retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		//// process materials
		//aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		//// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
		//// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
		//// Same applies to other texture as the following list summarizes:
		//// diffuse: texture_diffuseN
		//// specular: texture_specularN
		//// normal: texture_normalN

		//// 1. diffuse maps
		//vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		//textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		//// 2. specular maps
		//vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		//textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		//// 3. normal maps
		//std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		//textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		//// 4. height maps
		//std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
		//textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		// return a mesh object created from the extracted mesh data
		Mesh * newMesh = new Mesh(vertexData, indices, vertexDescription);
		newMesh->m_numVertices = mesh->mNumVertices;
		return newMesh;
	}
}