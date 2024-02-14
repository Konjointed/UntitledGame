#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

#include <glad/glad.h>

#include "Core/Math.h"

struct Shader {
	GLenum mType;
	std::string mFilepath;
	GLuint mId;
	std::filesystem::file_time_type mLastModified;
};

struct ShaderProgram {
	GLuint mId;
	std::vector<Shader> mShaders;

	void AddShader(GLenum type, const std::string filepath);
	Shader Compile(GLenum type, const std::string filepath);
	void Link();
	bool Reload();

	void SetUniformInt(const std::string& name, int value);
	void SetUniformFloat(const std::string& name, float value);
	void SetUniform(const std::string& name, const glm::vec3& value);
	void SetUniform(const std::string& name, const glm::mat4& value);
};

void LoadShaderProgram(const std::string name, const std::string vertexShaderPath, const std::string fragShaderPath, const std::string geomShaderPath = ""); 

#endif 