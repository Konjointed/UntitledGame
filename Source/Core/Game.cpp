#include "Game.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h>

#include <stb_image.h>

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
EventManager gEventManager;
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
			ImGui_ImplSDL2_ProcessEvent(&event);
			processSDLEvent(event);
		}

		float time = SDL_GetTicks() / 1000.0f;
		float timestep = time - lastFrameTime;
		lastFrameTime = time;

		UpdateScene(timestep);
		Renderer::Render(timestep);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		//ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		//ImGui::ShowDemoWindow();

		ImGui::Begin("Debug");
		static bool wireframe = false;
		if (ImGui::Checkbox("Wireframe Mode", &wireframe))
		{
			if (wireframe)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Enable wireframe mode
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Disable wireframe mode (default)
		}
		ImGui::End();

		ImGui::Begin("Scene Texture Viewer");

		static const char* textureTypes[] = { "Default", "Depth" };
		static int selectedItem = 0;
		ImGui::Combo("Texture Type", &selectedItem, textureTypes, IM_ARRAYSIZE(textureTypes));

		GLuint textureID = 0;
		switch (selectedItem)
		{
		case 0:
			textureID = renderData.mScreenColorTexture;
			break;
		case 1:
			Renderer::RenderDepthToColorTexture(0);
			textureID = renderData.mDepthDebugColorTexture;
			break;
		}

		float cameraAspectRatio = gScene.camera.get()->GetAspectRatio();
		ImVec2 availableSize = ImGui::GetContentRegionAvail();
		float availableAspectRatio = availableSize.x / availableSize.y;

		ImVec2 textureSize;
		if (availableAspectRatio > cameraAspectRatio) {
			// If the window is wider than the camera's aspect ratio, fit the height and adjust the width
			textureSize.y = availableSize.y;
			textureSize.x = availableSize.y * cameraAspectRatio;
		}
		else {
			// If the window is taller than the camera's aspect ratio, fit the width and adjust the height
			textureSize.x = availableSize.x;
			textureSize.y = availableSize.x / cameraAspectRatio;
		}

		ImVec2 uv0 = ImVec2(0.0f, 1.0f); // Bottom-left
		ImVec2 uv1 = ImVec2(1.0f, 0.0f); // Top-right
		ImGui::Image((void*)(intptr_t)textureID, textureSize, uv0, uv1);

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// for imgui docking feature
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
			SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
		}

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
	//-----------------------------------------------------------------------------
	// ImGUI
	//-----------------------------------------------------------------------------
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(m_window, m_glContext);
	ImGui_ImplOpenGL3_Init();

	return true;
}

void Game::shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

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
				gEventManager.Fire<WindowResizeEvent>(newWidth, newHeight);
			}
			else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
				m_quit = true;
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