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
struct WindowResizeEvent;

enum DepthResolution {
	LOW = 512,
	MEDIUM = 1024,
	HIGH = 2048,
	ULTRA = 4096,
	EXTREME = 8192
};

struct RendererData {
	int mWindowWidth = 1280;
	int mWindowHeight = 720;

	const glm::vec3 mLightDirection = gScene.lightDirection;
	const unsigned int mDepthMapResolution = DepthResolution::ULTRA;
	unsigned int mLightFBO; // -> mShadowMapFBO
	unsigned int mLightDepthMaps; // -> mShadowMapTexture
	unsigned int mMatricesUBO;
	std::vector<float> mShadowCascadeLevels;

	unsigned int mScreenFBO;
	unsigned int mScreenColorTexture;
	unsigned int mScreenRBO;

	unsigned int mDepthDebugFBO;
	unsigned int mDepthDebugColorTexture;

};

class Renderer {
public:
	static void Init();
	static void Resize(int windowWidth, int windowHeight);
	static void Render(float timestep);
	static void RenderDepthToColorTexture(int layer);
private:
	static void onWindowResize(const WindowResizeEvent& event);
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