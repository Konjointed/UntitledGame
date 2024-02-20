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
	bool mQuit = false;
	SDL_Window* mWindow = nullptr;
	SDL_GLContext mGlContext;
};

extern Game gGame;

#endif 