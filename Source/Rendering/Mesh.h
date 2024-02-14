#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>

#include <Core/Math.h>

struct aiMesh;
struct aiScene;
struct aiNode;

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	glm::vec3 color;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

struct Mesh {
	unsigned int vao, vbo, ebo;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Mesh> subMeshes;
};

class MeshLoader {
public:
	static void Load(const std::string& filepath, const std::string& name);
private:
	static Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	static std::vector<Mesh> processNode(aiNode* node, const aiScene* scene);
};

void LoadMesh(const std::string& filepath, const std::string& name);

#endif 