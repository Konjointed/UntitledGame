#ifndef RESOURCES_H
#define RESOURCES_H

#include <string>
#include <map>

struct Mesh;
struct ShaderProgram;
struct Texture;
//struct Material;

struct Resources {
	std::map<std::string, Mesh> mMeshes;
	std::map<std::string, ShaderProgram> mShaderPrograms;
	std::map<std::string, Texture> mTextures;
	//std::map<std::string, Material> mMaterials;
};

extern Resources gResources;

#endif 