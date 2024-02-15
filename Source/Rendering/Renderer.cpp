#include "Renderer.h"

#include <glad/glad.h>

#include "Log/Logger.h"
#include "Core/Resources.h"
#include "Rendering/Mesh.h"
#include "Rendering/Shader.h"
#include "Rendering/Texture.h"
#include "Scene/Scene.h"

//void Render() {
//	shadowPass();
//	lightingPass();
//}
//
//void shadowPass() {
//	// depth buffer stuff
//	// ...
//
//
//}
//
//lightingPass() {
//	// lighting stuff
//	// ...
//
//
//}


//void RenderPass() {
//	// Bind the shadow shader
//	// Set uniforms
//	// ...
//
//	// Render scene objects
//	for (Object sceneObj : sceneObjs) {
//		if sceneObject is static {
//			// Apply static shader object
//			// Set uniforms
//			// ...
//		}
//		else if sceneObject is dynamic{
//			// Apply dynamic shader object
//			// Set uniforms
//			// ...
//		}
//	}
//}

// TODO: Create a file with util/helper functions to make he buffers n shit
void Renderer::Init()
{
	renderData.mShadowCascadeLevels = { gScene.camera.get()->GetFarPlane() / 50.0f, gScene.camera.get()->GetFarPlane() / 25.0f, gScene.camera.get()->GetFarPlane() / 10.0f, gScene.camera.get()->GetFarPlane() / 2.0f };
	////-----------------------------------------------------------------------------
	//// Configure light frame buffer
	////-----------------------------------------------------------------------------
	glGenFramebuffers(1, &renderData.mLightFrameBuffer);

	glGenTextures(1, &renderData.mLightDepthMaps);
	glBindTexture(GL_TEXTURE_2D_ARRAY, renderData.mLightDepthMaps);
	glTexImage3D(
		GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, 
		renderData.mDepthMapResolution, renderData.mDepthMapResolution, int(renderData.mShadowCascadeLevels.size()) + 1,
		0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

	glBindFramebuffer(GL_FRAMEBUFFER, renderData.mLightFrameBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, renderData.mLightDepthMaps, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		spdlog::error("RENDERER::INIT: Framebuffer is not complete!");
		throw 0;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	////-----------------------------------------------------------------------------
	//// Configure uniform buffer
	////-----------------------------------------------------------------------------
	glGenBuffers(1, &renderData.mMatricesUniformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, renderData.mMatricesUniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, renderData.mMatricesUniformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	////-----------------------------------------------------------------------------
	//// Shader configuration
	////-----------------------------------------------------------------------------
	ShaderProgram& program = gResources.mShaderPrograms.at("shadow");
	glUseProgram(program.mId);
	program.SetUniformInt("diffuseTexture", 0);
	program.SetUniformInt("shadowMap", 1);
}

void Renderer::Render(float timestep) {
	ShaderProgram& program = gResources.mShaderPrograms.at("wind");
	glUseProgram(program.mId);
	program.SetUniform("projection", gScene.camera.get()->GetProjection());
	program.SetUniform("view", gScene.camera.get()->GetView());
	program.SetUniformFloat("time", timestep);
	renderScene(program);

	shadowPass();
	lightingPass();
}

void Renderer::renderScene(ShaderProgram program)
{
	for (auto& object : gScene.objects) {
		glm::vec3 position = object->GetPosition();
		glm::vec3 rotation = object->GetRotation();
		glm::vec3 scale = object->GetScale();

		glm::mat4 model = glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0, 1, 0)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0, 0, 1)) *
			glm::scale(glm::mat4(1.0f), scale);

		program.SetUniform("model", model);

		// Bind texture
		if (!object->GetTexture().empty()) {
			Texture texture = gResources.mTextures.at(object->GetTexture());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture.mId);
		}

		// Bind mesh
		if (!object->GetMesh().empty()) {
			Mesh mesh = gResources.mMeshes.at(object->GetMesh());
			glBindVertexArray(mesh.vao);
			glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}
}

void Renderer::shadowPass()
{
	//-----------------------------------------------------------------------------
	// 0. Uniform buffer setup
	//-----------------------------------------------------------------------------
	const auto lightMatrices = getLightSpaceMatrices();
	glBindBuffer(GL_UNIFORM_BUFFER, renderData.mMatricesUniformBuffer);
	for (size_t i = 0; i < lightMatrices.size(); ++i)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	//-----------------------------------------------------------------------------
	// 1. Render depth of scene to texture (from light's perspective)
	//-----------------------------------------------------------------------------
	ShaderProgram& program = gResources.mShaderPrograms.at("shadowDepth");
	glUseProgram(program.mId);

	glBindFramebuffer(GL_FRAMEBUFFER, renderData.mLightFrameBuffer);
	glViewport(0, 0, renderData.mDepthMapResolution, renderData.mDepthMapResolution);
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);  // peter panning

	renderScene(program);

	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//-----------------------------------------------------------------------------
	// Reset viewport
	//-----------------------------------------------------------------------------
	glViewport(0, 0, 1280, 720);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::lightingPass()
{ 
	//-----------------------------------------------------------------------------
	// 2. Render scene as normal using the generated depth/shadow map  
	//-----------------------------------------------------------------------------
	glViewport(0, 0, 1280, 720);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ShaderProgram& program = gResources.mShaderPrograms.at("shadow");
	glUseProgram(program.mId);
	program.SetUniform("projection", gScene.camera.get()->GetProjection());
	program.SetUniform("view", gScene.camera.get()->GetView()); 
	//-----------------------------------------------------------------------------
	// Set light uniforms
	//-----------------------------------------------------------------------------
	program.SetUniform("viewPos", gScene.camera.get()->GetPosition()); 
	program.SetUniform("lightDir", renderData.mLightDirection);
	program.SetUniformFloat("farPlane", gScene.camera.get()->GetFarPlane());
	program.SetUniformInt("cascadeCount", renderData.mShadowCascadeLevels.size());
	for (size_t i = 0; i < renderData.mShadowCascadeLevels.size(); ++i)
	{
		program.SetUniformFloat("cascadePlaneDistances[" + std::to_string(i) + "]", renderData.mShadowCascadeLevels[i]);
	}
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, renderData.mLightDepthMaps);
	
	renderScene(program);
}

std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview)
{
	const auto inv = glm::inverse(projview);

	std::vector<glm::vec4> frustumCorners;
	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}

	return frustumCorners;
}

std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
	return getFrustumCornersWorldSpace(proj * view);
}

glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane)
{
	const auto projection = glm::perspective(glm::radians(70.0f), (float)1280 / (float)720, nearPlane, farPlane);
	const auto corners = getFrustumCornersWorldSpace(projection, gScene.camera.get()->GetView());

	glm::vec3 center = glm::vec3(0, 0, 0);
	for (const auto& v : corners)
	{
		center += glm::vec3(v);
	}
	center /= corners.size();

	const auto lightView = glm::lookAt(center + renderData.mLightDirection, center, glm::vec3(0.0f, 1.0f, 0.0f));

	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();
	for (const auto& v : corners)
	{
		const auto trf = lightView * v;
		minX = std::min(minX, trf.x);
		maxX = std::max(maxX, trf.x);
		minY = std::min(minY, trf.y);
		maxY = std::max(maxY, trf.y);
		minZ = std::min(minZ, trf.z);
		maxZ = std::max(maxZ, trf.z);
	}

	// Tune this parameter according to the scene
	constexpr float zMult = 10.0f;
	if (minZ < 0)
	{
		minZ *= zMult;
	}
	else
	{
		minZ /= zMult;
	}
	if (maxZ < 0)
	{
		maxZ /= zMult;
	}
	else
	{
		maxZ *= zMult;
	}

	const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
	return lightProjection * lightView;
}

std::vector<glm::mat4> getLightSpaceMatrices()
{
	std::vector<glm::mat4> ret;
	for (size_t i = 0; i < renderData.mShadowCascadeLevels.size() + 1; ++i)
	{
		if (i == 0)
		{
			ret.push_back(getLightSpaceMatrix(gScene.camera.get()->GetNearPlane(), renderData.mShadowCascadeLevels[i]));
		}
		else if (i < renderData.mShadowCascadeLevels.size())
		{
			ret.push_back(getLightSpaceMatrix(renderData.mShadowCascadeLevels[i - 1], renderData.mShadowCascadeLevels[i]));
		}
		else
		{
			ret.push_back(getLightSpaceMatrix(renderData.mShadowCascadeLevels[i - 1], gScene.camera.get()->GetFarPlane()));
		}
	}
	return ret;
}