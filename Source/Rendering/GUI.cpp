#include "GUI.h"

#include <SDL.h>
#include <SDL_opengl.h>

void GUI::init(SDL_Window* window, SDL_GLContext context) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init();
}

void GUI::shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void GUI::beginFrame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

void GUI::draw() {
	ImGui::ShowDemoWindow();
}

void GUI::endFrame() {
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
}

//ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
//ImGui::ShowDemoWindow();

//ImGui::Begin("Debug");
//static bool wireframe = false;
//if (ImGui::Checkbox("Wireframe Mode", &wireframe))
//{
//	if (wireframe)
//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Enable wireframe mode
//	else
//		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Disable wireframe mode (default)
//}
//ImGui::End();

//ImGui::Begin("Scene Texture Viewer");

//static const char* textureTypes[] = { "Default", "Depth" };
//static int selectedItem = 0;
//ImGui::Combo("Texture Type", &selectedItem, textureTypes, IM_ARRAYSIZE(textureTypes));

//GLuint textureID = 0;
//switch (selectedItem)
//{
//case 0:
//	textureID = renderData.mScreenColorTexture;
//	break;
//case 1:
//	Renderer::RenderDepthToColorTexture(0);
//	textureID = renderData.mDepthDebugColorTexture;
//	break;
//}

//float cameraAspectRatio = gScene.camera.get()->GetAspectRatio();
//ImVec2 availableSize = ImGui::GetContentRegionAvail();
//float availableAspectRatio = availableSize.x / availableSize.y;

//ImVec2 textureSize;
//if (availableAspectRatio > cameraAspectRatio) {
//	// If the window is wider than the camera's aspect ratio, fit the height and adjust the width
//	textureSize.y = availableSize.y;
//	textureSize.x = availableSize.y * cameraAspectRatio;
//}
//else {
//	// If the window is taller than the camera's aspect ratio, fit the width and adjust the height
//	textureSize.x = availableSize.x;
//	textureSize.y = availableSize.x / cameraAspectRatio;
//}

//ImVec2 uv0 = ImVec2(0.0f, 1.0f); // Bottom-left
//ImVec2 uv1 = ImVec2(1.0f, 0.0f); // Top-right
//ImGui::Image((void*)(intptr_t)textureID, textureSize, uv0, uv1);

//ImGui::End();