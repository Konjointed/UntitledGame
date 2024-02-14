#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <unordered_map>

#include <glm/glm.hpp>
#include <SDL2/SDL_keycode.h>
#include <SDL_mouse.h>

struct KeyPressEvent {
	SDL_Keycode m_keycode;
};

struct KeyReleaseEvent {
	SDL_Keycode m_keycode;
};

struct ButtonPressEvent {
	int m_button;
};

struct ButtonReleaseEvent {
	int m_button;
};

struct MouseMoveEvent {
	int m_mouseX;
	int m_mouseY;
};

struct MouseWheelEvent {
	int m_x;
	int m_y; 
};

class InputManager {
public:
	void StartUp();
	void ShutDown();

	void Update();

	bool IsKeyDown(int key);
	bool IsMouseButtonDown(int button);

	glm::vec2 GetMouseDelta();
	int GetMouseWheelDelta();
private:
	int getDeltaMouseX();
	int getDeltaMouseY();
	int getMouseWheel();

	void onKeyPressed(const KeyPressEvent& event);
	void onKeyReleased(const KeyReleaseEvent& event);
	void onMouseButtonPressed(const ButtonPressEvent& event);
	void onMouseButtonReleased(const ButtonReleaseEvent& event);
	void onMouseMoved(const MouseMoveEvent& event);
	void onMouseWheel(const MouseWheelEvent& event);
private:
	int m_currentMouseX, m_currentMouseY;
	int m_previousMouseX, m_previousMouseY;
	int m_mouseWheel = 0;

	std::unordered_map<int, bool> m_keyState;
	std::unordered_map<int, bool> m_buttonState;
};

extern InputManager gInputManager;

#endif 