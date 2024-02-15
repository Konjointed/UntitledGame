#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>

#include "Core/Math.h"
#include "Core/Resources.h"

struct Material {
    std::string shaderName;
    std::string diffuseTextureName;
    std::string normalTextureName;
    std::string specularTextureName;
    glm::vec3 ambientColor;
    glm::vec3 diffuseColor;
    glm::vec3 specularColor;
    float shininess;

    Material(const std::string& shader = "", const std::string& diffuse = "", const std::string& normal = "", const std::string& specular = "", const glm::vec3& ambient = glm::vec3(1.0f), const glm::vec3& diffuseColor = glm::vec3(1.0f), const glm::vec3& specularColor = glm::vec3(1.0f), float shininessVal = 32.0f)
        : shaderName(shader), diffuseTextureName(diffuse), normalTextureName(normal), specularTextureName(specular), ambientColor(ambient), diffuseColor(diffuseColor), specularColor(specularColor), shininess(shininessVal) {}
};

//void LoadMaterial(const std::string& name, const std::string& shader, const std::string& diffuse, const std::string& normal, const std::string& specular, const glm::vec3& ambient, const glm::vec3& diffuseColor, const glm::vec3& specularColor, float shininess) {
//    Material mat(shader, diffuse, normal, specular, ambient, diffuseColor, specularColor, shininess);
//    gResources.mMaterials.emplace(name, std::move(mat));
//}

#endif 