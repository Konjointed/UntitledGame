#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <string>

#include "Core/Math.h"

enum class ObjectType {
	OBJECT_TYPE_NONE,
	OBJECT_TYPE_STATIC,
	OBJECT_TYPE_DYNAMIC
};

class GameObject {
public:
	GameObject(const std::string& name, const std::string& meshName, const std::string& textureName,
		const glm::vec3& position = glm::vec3(0.0f),
		const glm::vec3& rotation = glm::vec3(0.0f),
		const glm::vec3& scale = glm::vec3(1.0f));
	virtual ~GameObject() = default;

	void Print();

	ObjectType GetStaticType();
	std::string GetMesh();
	std::string GetTexture();
	glm::vec3 GetPosition();
	glm::vec3 GetRotation();
	glm::vec3 GetScale();
private:
	ObjectType mType;

	std::string mName;
	std::string mMeshName;
	std::string mTextureName;
	std::string mShaderName;

	glm::vec3 mPosition;
	glm::vec3 mRotation;
	glm::vec3 mScale;
};

#endif 