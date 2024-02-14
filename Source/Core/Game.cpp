#include "Game.h"

#include "Log/Logger.h"
#include "Core/Resources.h"
#include "Event/EventManager.h"
#include "Input/InputManager.h"
#include "Rendering/Renderer.h"
#include "Scene/Scene.h"
#include "Rendering/Mesh.h"
#include "Rendering/Shader.h"
#include "Rendering/Texture.h"

Game gGame;
Resources gResources;
InputManager gInputManager;
Scene gScene;
RendererData renderData;

int Game::Run(const char* title, int width, int height, bool fullscreen)
{
	if (!initialize(title, width, height, fullscreen)) {
		return -1;
	}

	gInputManager.StartUp();

	glEnable(GL_DEPTH_TEST);

	loadResources();

	CreateScene();
	Renderer::Init();

	float lastFrameTime = 0.0f;
	while (!m_quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			processSDLEvent(event);
		}

		float time = SDL_GetTicks() / 1000.0f;
		float timestep = time - lastFrameTime;
		lastFrameTime = time;

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		UpdateScene(timestep);
		Renderer::RenderScene();

		//if (gResources.mShaderPrograms["shadow"].Reload()) {
		//	Renderer::Init();
		//}
		//gResources.mShaderPrograms["depth"].Reload();

		gInputManager.Update();

		SDL_GL_SwapWindow(m_window);
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
	m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, windowFlags);
	if (!m_window) {
		spdlog::error("SDL Create Window {}", SDL_GetError());
		return false;
	}
	//-----------------------------------------------------------------------------
	// Create opengl context
	//-----------------------------------------------------------------------------
	m_glContext = SDL_GL_CreateContext(m_window);
	if (!m_glContext) {
		spdlog::error(" SDL GL Context {}", SDL_GetError());
		SDL_DestroyWindow(m_window);
		SDL_Quit();
		return false;
	}
	//-----------------------------------------------------------------------------
	// Load opengl functions and pointers
	//-----------------------------------------------------------------------------
	if (!gladLoadGL()) {
		spdlog::error("GLAD Init {}", SDL_GetError());
		SDL_GL_DeleteContext(m_glContext);
		SDL_DestroyWindow(m_window);
		SDL_Quit();
		return false;
	}

	return true;
}

void Game::shutdown()
{
	SDL_GL_DeleteContext(m_glContext);
	SDL_DestroyWindow(m_window);
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
			}
			break;
		}
		case SDL_QUIT:
		{
			m_quit = true;
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
	LoadShaderProgram("default", "Resources/Shaders/default.vert", "Resources/Shaders/default.frag");
	LoadShaderProgram("shadow", "Resources/Shaders/shadowMapping.vert", "Resources/Shaders/shadowMapping.frag");
	LoadShaderProgram("depth", "Resources/Shaders/shadowMappingDepth.vert", "Resources/Shaders/shadowMappingDepth.frag", "Resources/Shaders/shadowMappingDepth.geom");
	
	LoadMesh("Resources/Meshes/Maria/Maria J J Ong.dae", "maria");
	LoadMesh("Resources/Meshes/suzanne.obj", "suzanne");
	LoadMesh("Resources/Meshes/cube.obj", "cube");

	LoadTexture("Resources/Textures/wood.png", "wood");
	LoadTexture("Resources/Textures/brickwall.jpg", "brick");
}