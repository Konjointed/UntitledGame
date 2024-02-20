#include "Game.h"

#include <stb_image.h>

#include "Log/Logger.h"
#include "Core/Resources.h"
#include "Event/EventManager.h"
#include "Input/InputManager.h"
#include "Rendering/Renderer.h"
#include "Rendering/GUI.h"
#include "Scene/Scene.h"
#include "Rendering/Mesh.h"
#include "Rendering/Shader.h"
#include "Rendering/Texture.h"

Game gGame;
Resources gResources;
EventManager gEventManager;
InputManager gInputManager;
Scene gScene;

int Game::Run(const char* title, int width, int height, bool fullscreen)
{
	if (!initialize(title, width, height, fullscreen)) {
		return -1;
	}

	gInputManager.StartUp();

	glEnable(GL_DEPTH_TEST);

	loadResources();

	CreateScene();
	Renderer::init();

	float lastFrameTime = 0.0f;
	while (!mQuit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			processSDLEvent(event);
		}

		float time = SDL_GetTicks() / 1000.0f;
		float timestep = time - lastFrameTime;
		lastFrameTime = time;

		UpdateScene(timestep);
		Renderer::render();
		GUI::beginFrame();
		GUI::draw();
		GUI::endFrame();

		gInputManager.Update();

		SDL_GL_SwapWindow(mWindow);
	}

	shutdown();

	return 0;
}

bool Game::initialize(const char* title, int width, int height, bool fullscreen)
{
	//-----------------------------------------------------------------------------
	// Initialzie SDL
	//-----------------------------------------------------------------------------
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		spdlog::error("SDL Init {}", SDL_GetError());
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	//-----------------------------------------------------------------------------
	// Create sdl window
	//-----------------------------------------------------------------------------
	const Uint32 windowFlags = (SDL_WINDOW_OPENGL | (fullscreen ? SDL_WINDOW_RESIZABLE : 0));
	mWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, windowFlags);
	if (!mWindow) {
		spdlog::error("SDL Create Window {}", SDL_GetError());
		return false;
	}
	//-----------------------------------------------------------------------------
	// Create opengl context
	//-----------------------------------------------------------------------------
	mGlContext = SDL_GL_CreateContext(mWindow);
	if (!mGlContext) {
		spdlog::error(" SDL GL Context {}", SDL_GetError());
		SDL_DestroyWindow(mWindow);
		SDL_Quit();
		return false;
	}
	//-----------------------------------------------------------------------------
	// Load opengl functions and pointers
	//-----------------------------------------------------------------------------
	if (!gladLoadGL()) {
		spdlog::error("GLAD Init {}", SDL_GetError());
		SDL_GL_DeleteContext(mGlContext);
		SDL_DestroyWindow(mWindow);
		SDL_Quit();
		return false;
	}
	//-----------------------------------------------------------------------------
	// ImGUI
	//-----------------------------------------------------------------------------
	GUI::init(mWindow, mGlContext);

	return true;
}

void Game::shutdown()
{
	GUI::shutdown();

	SDL_GL_DeleteContext(mGlContext);
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}

void Game::processSDLEvent(SDL_Event& event)
{
	switch (event.type) 
	{
		case SDL_WINDOWEVENT:
		{
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				int newWidth = event.window.data1;
				int newHeight = event.window.data2;
				glViewport(0, 0, newWidth, newHeight);
				gEventManager.Fire<WindowResizeEvent>(newWidth, newHeight);
			}
			else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
				mQuit = true;
			}
			break;
		}
		case SDL_QUIT:
		{
			mQuit = true;
			break;
		}
		case SDL_KEYDOWN:
		{
			gEventManager.Fire<KeyPressEvent>(event.key.keysym.sym);
			break;
		}
		case SDL_KEYUP:
		{
			gEventManager.Fire<KeyReleaseEvent>(event.key.keysym.sym);
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			gEventManager.Fire<ButtonPressEvent>(event.button.button);
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			gEventManager.Fire<ButtonReleaseEvent>(event.button.button);
			break;
		}
		case SDL_MOUSEWHEEL:
		{
			gEventManager.Fire<MouseWheelEvent>(event.wheel.x, event.wheel.y);
			break;
		}
		case SDL_MOUSEMOTION:
		{
			gEventManager.Fire<MouseMoveEvent>(event.motion.x, event.motion.y);
			break;
		}
	}
}

void Game::loadResources() {
	LoadShaderProgram("shadow", "Resources/Shaders/CSMPhong.vert", "Resources/Shaders/CSMPhong.frag");
	LoadShaderProgram("shadowDepth", "Resources/Shaders/shadowMappingDepth.vert", "Resources/Shaders/shadowMappingDepth.frag", "Resources/Shaders/shadowMappingDepth.geom");
	LoadShaderProgram("screen", "Resources/Shaders/screen.vert", "Resources/Shaders/screen.frag");
	LoadShaderProgram("debugDepth", "Resources/Shaders/debugDepth.vert", "Resources/Shaders/debugDepth.frag");

	//LoadShaderProgram("wind", "Resources/Shaders/Wind.vert", "Resources/Shaders/CSMPhong.frag");
	//LoadShaderProgram("shadowDepthWind", "Resources/Shaders/ShadowDepthWind.vert", "Resources/Shaders/shadowMappingDepth.frag", "Resources/Shaders/shadowMappingDepth.geom");
	
	LoadMesh("Resources/Meshes/Shotgun.fbx", "gun");
	LoadMesh("Resources/Meshes/Rock/Rock.obj", "rock");
	LoadMesh("Resources/Meshes/Maria/Maria J J Ong.dae", "maria");
	LoadMesh("Resources/Meshes/suzanne.obj", "suzanne");
	LoadMesh("Resources/Meshes/cube.obj", "cube");

	LoadTexture("Resources/Textures/wood.png", "wood");
	LoadTexture("Resources/Textures/brickwall.jpg", "brick");
}

//LoadShaderProgram("CSMPhong", "Resources/Shaders/CSMPhong.vert", "Resources/Shaders/CSMPhong.frag");
//LoadShaderProgram("ShadowDepth", "Resources/Shaders/ShadowDepth.vert", "Resources/Shaders/ShadowDepth.frag", "Resources/Shaders/ShadowDepth.geom");
//
//LoadShaderProgram("CSMPhongWind", "Resources/Shaders/CSMPhongWind.vert", "Resources/Shaders/CSMPhong.frag");
//LoadShaderProgram("ShadowDepthWind", "Resources/Shaders/ShadowDepthWind.vert", "Resources/Shaders/ShadowDepth.frag", "Resources/Shaders/ShadowDepth.geom");