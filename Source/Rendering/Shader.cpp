#include "Shader.h"

#include <sstream>
#include <fstream>

#include "Log/Logger.h"
#include "Core/Resources.h"

void ShaderProgram::AddShader(GLenum type, const std::string filepath)
{
	Shader shader = Compile(type, filepath);
	mShaders.push_back(shader);
}

Shader ShaderProgram::Compile(GLenum type, const std::string filepath)
{
	Shader result;
	result.mType = type;
	result.mFilepath = filepath;

	// Load file content 
	std::string shaderSource;
	try
	{
		std::ifstream shaderFile(filepath);
		std::stringstream shaderStream;
		shaderStream << shaderFile.rdbuf();
		shaderFile.close();
		shaderSource = shaderStream.str();
	}
	catch (std::exception e)
	{
		spdlog::error("SHADER::COMPILESHADER: Failed to read shader files");
	}

	// Create shader and compile it
	result.mId = glCreateShader(type);
	const GLchar* source = shaderSource.c_str();
	glShaderSource(result.mId, 1, &source, nullptr);
	glCompileShader(result.mId);

	GLint compiled = GL_FALSE;
	glGetShaderiv(result.mId, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(result.mId, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* infoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(result.mId, infoLogLength, NULL, infoLog);
		spdlog::error("SHADER::COMPILESHADER: Compilation failed {}", infoLog);
		delete[] infoLog;
		result.mId = 0;
	}

	return result;
}

void ShaderProgram::Link()
{
	mId = glCreateProgram();

	for (auto& shader : mShaders)
	{
		glAttachShader(mId, shader.mId);
	}
	glLinkProgram(mId);

	GLint success;
	glGetProgramiv(mId, GL_LINK_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetProgramInfoLog(mId, 512, NULL, infoLog);
		spdlog::error("SHADER::LINKSHADERPROGRAM: Linking failed {}", infoLog);
	}

	for (auto& shader : mShaders)
	{
		glDetachShader(mId, shader.mId);
		glDeleteShader(shader.mId);
	}
}

bool ShaderProgram::Reload() {
	bool needsReloading = false;
	for (auto& shader : mShaders) {
		auto currentModTime = std::filesystem::last_write_time(shader.mFilepath);
		if (shader.mLastModified < currentModTime) {
			needsReloading = true;
			shader.mLastModified = currentModTime; // Update the last modification time
		}
	}

	if (needsReloading) {
		spdlog::info("needs reloading");

		// Detach existing shaders
		for (auto& shader : mShaders) {
			glDetachShader(mId, shader.mId);
			glDeleteShader(shader.mId);
		}

		// Recompile shaders
		for (auto& shader : mShaders) {
			shader.mId = Compile(shader.mType, shader.mFilepath).mId;
			glAttachShader(mId, shader.mId);
		}

		// Relink the program
		Link();

		return true;
	}

	return false;
}

void ShaderProgram::SetUniformInt(const std::string& name, int value)
{
	GLint location = glGetUniformLocation(mId, name.c_str());
	glUniform1i(location, value);
}

void ShaderProgram::SetUniformFloat(const std::string& name, float value)
{
	GLint location = glGetUniformLocation(mId, name.c_str());
	glUniform1f(location, value);
}

void ShaderProgram::SetUniform(const std::string& name, const glm::vec3& value)
{
	GLint location = glGetUniformLocation(mId, name.c_str());
	glUniform3fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::SetUniform(const std::string& name, const glm::mat4& value)
{
	GLint location = glGetUniformLocation(mId, name.c_str());
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void LoadShaderProgram(const std::string name, const std::string vertexShaderPath, const std::string fragShaderPath, const std::string geomShaderPath)
{
	ShaderProgram program;

	program.AddShader(GL_VERTEX_SHADER, vertexShaderPath);
	program.AddShader(GL_FRAGMENT_SHADER, fragShaderPath);
	if (!geomShaderPath.empty()) {
		program.AddShader(GL_GEOMETRY_SHADER, geomShaderPath);
	}

	program.Link();

	gResources.mShaderPrograms.emplace(name, program);
}