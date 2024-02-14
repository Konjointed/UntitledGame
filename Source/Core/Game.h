#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <SDL.h>
#include <SDL_opengl.h>

struct Resources;

class Game {
public:
	int Run(const char* title, int width, int height, bool fullscreen);
private:
	bool initialize(const char* title, int width, int height, bool fullscreen);
	void shutdown();
	void processSDLEvent(SDL_Event& event);
	void loadResources();
private:
	bool m_quit = false;
	SDL_Window* m_window = nullptr;
	SDL_GLContext m_glContext = nullptr;
};

extern Game gGame;

#endif 