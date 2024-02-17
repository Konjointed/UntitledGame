/*
Pipeline: An ordered chain of computational stages, each with a specific purpose operating on a stream of input data 
// Shadows
// Lighting
// Post Process
*/

/*
Step 1: Associate objects with a type Static or Dynamic
	- Static for non animated things like trees, rocks, etc
	- Dynamic for animated things like a character
Step 2: Associate object with a shader
	- Object stores a handle (or id) to a shader in some database
Step 3: During rendering apply a shadow shader to everything
Step 4: Sort objects based on shader so you're not constantly switching between shaders
Step 5: Apply the shader associated with the object
*/

#ifndef RENDERER_H
#define RENDERER_H

#include <vector>

#include "Core/Math.h"
#include "Scene/Scene.h"

struct ShaderProgram;

enum DepthResolution {
	LOW = 512,
	MEDIUM = 1024,
	HIGH = 2048,
	ULTRA = 4096,
	EXTREME = 8192
};

struct RendererData {
	const glm::vec3 mLightDirection = gScene.lightDirection;
	const unsigned int mDepthMapResolution = DepthResolution::ULTRA;
	unsigned int mLightFrameBuffer;
	unsigned int mLightDepthMaps;
	unsigned int mMatricesUniformBuffer;
	std::vector<float> mShadowCascadeLevels;

	// Post-processing framebuffer and textures
	unsigned int ppFrameBuffer; // Post-processing Framebuffer
	unsigned int ppTextureColor; // Texture attachment for color
};

class Renderer {
public:
	static void Init();
	static void Render(float timestep);
private:
	static void renderScene(ShaderProgram program);
	static void shadowPass();
	static void lightingPass();
};

void renderScreenQuad();
std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview);
std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);
glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane);
std::vector<glm::mat4> getLightSpaceMatrices();

extern RendererData renderData;

#endif 


/*
Up until now I've been using super basic shaders or things I've learned from learnopengl.com and I'd like to combine everything, but I don't fully understand how.
I thought it was as simple as apply X shader and set uniforms, apply Y shader and set uniforms, etc and you're good to go, but that doesn't seem to be the case.

void Renderer::Render() {
	shadowPass();
	lightingPass();
}

void Renderer::shadowPass() {
	// Uniform buffer setup
	// Render depth of scene
	// Render scene
}

void Renderer::lightingPass() {
	// Render scene with generated depth map
	// Set uniforms
	// Render scene
}
*/