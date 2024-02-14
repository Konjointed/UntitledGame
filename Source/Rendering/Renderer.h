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

enum Resolution {
	LOW = 512,
	MEDIUM = 1024,
	HIGH = 2048,
	ULTRA = 4096,
	EXTREME = 8192
};

// TODO: Make lightdir to the scene (and any other/future data)
struct RendererData {
	const glm::vec3 mLightDirection = glm::normalize(glm::vec3(20.0f, 50, 20.0f));
	const unsigned int mDepthMapResolution = Resolution::HIGH;
	unsigned int mLightFrameBuffer;
	unsigned int mLightDepthMaps;
	unsigned int mMatricesUniformBuffer;
	std::vector<float> mShadowCascadeLevels;
};

class Renderer {
public:
	static void Init();
	static void RenderScene();
private:
	static void shadowPass();
	static void lightingPass();
};

std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview);
std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);
glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane);
std::vector<glm::mat4> getLightSpaceMatrices();

extern RendererData renderData;

#endif 