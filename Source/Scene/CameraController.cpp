#include "CameraController.h"

#include "Input/InputManager.h"
#include "Scene/Camera.h"

void CameraController::Update(Camera& camera, float timestep)
{
	const float speed = 5.0f;
	const float sensitivity = 0.1f;

	if (gInputManager.IsMouseButtonDown(SDL_BUTTON_RIGHT)) {
		glm::vec2 mouseDelta = gInputManager.GetMouseDelta();
		camera.Pivot(mouseDelta.x, mouseDelta.y, sensitivity);
	}

	if (gInputManager.IsKeyDown(SDLK_w)) {
		camera.MoveForward(speed * timestep);
	}

	if (gInputManager.IsKeyDown(SDLK_s)) {
		camera.MoveBackward(speed * timestep);
	}

	if (gInputManager.IsKeyDown(SDLK_a)) {
		camera.StrafeLeft(speed * timestep);
	}

	if (gInputManager.IsKeyDown(SDLK_d)) {
		camera.StrafeRight(speed * timestep);
	}
}
