#include "Renderer.h"

#include <glad/glad.h>

#include "Log/Logger.h"
#include "Core/Resources.h"
#include "Event/EventManager.h"
#include "Rendering/Mesh.h"
#include "Rendering/Shader.h"
#include "Rendering/Texture.h"
#include "Rendering/Buffers.h"
#include "Scene/Scene.h"

//static float time = 0.0f;
//time += 0.01f;
//program.SetUniformFloat("time", time);

Renderer::RendererData Renderer::gRenderData;

void Renderer::init()
{
	gEventManager.Connect<WindowResizeEvent>(&Renderer::onWindowResize);

	gRenderData.mShadowCascadeLevels = { gScene.camera.get()->GetFarPlane() / 50.0f, gScene.camera.get()->GetFarPlane() / 25.0f, gScene.camera.get()->GetFarPlane() / 10.0f, gScene.camera.get()->GetFarPlane() / 2.0f };
	////-----------------------------------------------------------------------------
	//// Configure light framebuffer
	////-----------------------------------------------------------------------------
	gRenderData.mLightFBO = CreateFrameBuffer(true);

	// TODO: Understand this (is this an attachment?)
	gRenderData.mLightDepthMaps = CreateDepthTextureArray(gRenderData.mDepthMapResolution, gRenderData.mDepthMapResolution, gRenderData.mShadowCascadeLevels.size() + 1);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gRenderData.mLightDepthMaps, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	ValidateBuffer(gRenderData.mLightFBO);
	BindDefaultFramebuffer();
	////-----------------------------------------------------------------------------
	//// Configure uniformbuffer
	////-----------------------------------------------------------------------------
	// TODO: Understand this
	glGenBuffers(1, &gRenderData.mMatricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, gRenderData.mMatricesUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, gRenderData.mMatricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::resize(int windowWidth, int windowHeight) {
	//-----------------------------------------------------------------------------
	// Configure post-processing framebuffer
	//-----------------------------------------------------------------------------
	gRenderData.mScreenFBO = CreateFrameBuffer(true);

	gRenderData.mScreenColorTexture = CreateColorTextureAttachment(windowWidth, windowHeight);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gRenderData.mScreenColorTexture, 0);

	gRenderData.mScreenRBO = CreateRenderBufferAttachment(windowWidth, windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gRenderData.mScreenRBO);

	ValidateBuffer(gRenderData.mScreenFBO);
	BindDefaultFramebuffer();
	//-----------------------------------------------------------------------------
	// Configure debug depth framebuffer
	//-----------------------------------------------------------------------------
	gRenderData.mDepthDebugFBO = CreateFrameBuffer(true);

	gRenderData.mDepthDebugColorTexture = CreateColorTextureAttachment(windowWidth, windowHeight);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gRenderData.mDepthDebugColorTexture, 0);

	ValidateBuffer(gRenderData.mDepthDebugFBO);
	BindDefaultFramebuffer();
}

void Renderer::render() {
	shadowPass(); // Generate shadow maps

	BindFramebuffer(gRenderData.mScreenFBO);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	lightingPass();

	// Bind the default framebuffer and draw the screen quad with post-processing
	BindDefaultFramebuffer();
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Use the post-processing shader
	ShaderProgram& ppShader = gResources.mShaderPrograms.at("screen");
	glUseProgram(ppShader.mId);
	ppShader.SetUniformInt("screenTexture", 0);

	// Bind the texture of the offscreen framebuffer
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gRenderData.mScreenColorTexture);

	// Render the quad
	renderScreenQuad();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, gRenderData.mLightDepthMaps);

	glEnable(GL_DEPTH_TEST);
}

void Renderer::renderDepthToColorTexture(int layer)
{
	glBindFramebuffer(GL_FRAMEBUFFER, gRenderData.mDepthDebugFBO);
	glViewport(0, 0, gRenderData.mWindowWidth, gRenderData.mWindowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ShaderProgram& debugShader = gResources.mShaderPrograms.at("debugDepth");
	glUseProgram(debugShader.mId);

	debugShader.SetUniformInt("layer", layer);

	glActiveTexture(GL_TEXTURE0); // Use texture unit 0
	glBindTexture(GL_TEXTURE_2D_ARRAY, gRenderData.mLightDepthMaps);
	debugShader.SetUniformInt("shadowMap", 0);

	renderScreenQuad();

	// Reset OpenGL state
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind back to default framebuffer
	glViewport(0, 0, gRenderData.mWindowWidth, gRenderData.mWindowHeight); // Reset viewport to original dimensions
	glEnable(GL_DEPTH_TEST); // Ensure depth test is enabled for subsequent rendering
	glActiveTexture(GL_TEXTURE0); // Reset to default texture unit if necessary
	// Additional state resets can be added here as needed
}

void Renderer::shadowPass()
{
	//-----------------------------------------------------------------------------
	// 0. Uniform buffer setup
	//-----------------------------------------------------------------------------
	const auto lightMatrices = getLightSpaceMatrices();
	glBindBuffer(GL_UNIFORM_BUFFER, gRenderData.mMatricesUBO);
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

	BindFramebuffer(gRenderData.mLightFBO);
	glViewport(0, 0, gRenderData.mDepthMapResolution, gRenderData.mDepthMapResolution);
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);  // peter panning

	renderScene(program);

	glCullFace(GL_BACK);
	BindDefaultFramebuffer();
	//-----------------------------------------------------------------------------
	// Reset viewport
	//-----------------------------------------------------------------------------
	glViewport(0, 0, gRenderData.mWindowWidth, gRenderData.mWindowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::lightingPass()
{ 
	//-----------------------------------------------------------------------------
	// 2. Render scene as normal using the generated depth/shadow map  
	//-----------------------------------------------------------------------------
	glViewport(0, 0, gRenderData.mWindowWidth, gRenderData.mWindowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ShaderProgram& program = gResources.mShaderPrograms.at("shadow");
	glUseProgram(program.mId);
	program.SetUniform("projection", gScene.camera.get()->GetProjection());
	program.SetUniform("view", gScene.camera.get()->GetView()); 
	program.SetUniformInt("diffuseTexture", 0);
	program.SetUniformInt("shadowMap", 1);
	//-----------------------------------------------------------------------------
	// Set light uniforms
	//-----------------------------------------------------------------------------
	program.SetUniform("viewPos", gScene.camera.get()->GetPosition()); 
	program.SetUniform("lightDir", gRenderData.mLightDirection);
	program.SetUniformFloat("farPlane", gScene.camera.get()->GetFarPlane());
	program.SetUniformInt("cascadeCount", gRenderData.mShadowCascadeLevels.size());
	for (size_t i = 0; i < gRenderData.mShadowCascadeLevels.size(); ++i)
	{
		program.SetUniformFloat("cascadePlaneDistances[" + std::to_string(i) + "]", gRenderData.mShadowCascadeLevels[i]);
	}
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, gRenderData.mLightDepthMaps);

	renderScene(program);
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

void Renderer::onWindowResize(const WindowResizeEvent& event) {
	gRenderData.mWindowWidth = event.x;
	gRenderData.mWindowHeight = event.y;
	resize(event.x, event.y);
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void Renderer::renderScreenQuad() {
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

std::vector<glm::vec4> Renderer::getFrustumCornersWorldSpace(const glm::mat4& projview)
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

std::vector<glm::vec4> Renderer::getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
	return getFrustumCornersWorldSpace(proj * view);
}

glm::mat4 Renderer::getLightSpaceMatrix(const float nearPlane, const float farPlane)
{
	const auto projection = glm::perspective(glm::radians(70.0f), (float)gRenderData.mWindowWidth / (float)gRenderData.mWindowHeight, nearPlane, farPlane);
	const auto corners = getFrustumCornersWorldSpace(projection, gScene.camera.get()->GetView());

	glm::vec3 center = glm::vec3(0, 0, 0);
	for (const auto& v : corners)
	{
		center += glm::vec3(v);
	}
	center /= corners.size();

	const auto lightView = glm::lookAt(center + gRenderData.mLightDirection, center, glm::vec3(0.0f, 1.0f, 0.0f));

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

std::vector<glm::mat4> Renderer::getLightSpaceMatrices()
{
	std::vector<glm::mat4> ret;
	for (size_t i = 0; i < gRenderData.mShadowCascadeLevels.size() + 1; ++i)
	{
		if (i == 0)
		{
			ret.push_back(getLightSpaceMatrix(gScene.camera.get()->GetNearPlane(), gRenderData.mShadowCascadeLevels[i]));
		}
		else if (i < gRenderData.mShadowCascadeLevels.size())
		{
			ret.push_back(getLightSpaceMatrix(gRenderData.mShadowCascadeLevels[i - 1], gRenderData.mShadowCascadeLevels[i]));
		}
		else
		{
			ret.push_back(getLightSpaceMatrix(gRenderData.mShadowCascadeLevels[i - 1], gScene.camera.get()->GetFarPlane()));
		}
	}
	return ret;
}