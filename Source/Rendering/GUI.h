#ifndef GUI_H
#define GUI_H

#include <SDL_video.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h>

namespace GUI {
	void init(SDL_Window* window, SDL_GLContext context);
	void shutdown();
	void beginFrame();
	void draw();
	void endFrame();
}

#endif 