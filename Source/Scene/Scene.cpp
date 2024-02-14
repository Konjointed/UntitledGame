#include "Scene.h"

#include "Scene/CameraController.h"

void AddObject(GameObject* object) {
	object->Print();
	gScene.objects.push_back(std::unique_ptr<GameObject>(object));
}

void CreateScene() {
	gScene.camera = std::make_unique<Camera>();
	gScene.camera.get()->SetController(new CameraController);

	//AddObject(new GameObject("Maria", "maria", "wood"));
	AddObject(new GameObject("Suzanne", "suzanne", "wood", glm::vec3(5.0f, 0.0f, 0.0f)));
	AddObject(new GameObject("Cube", "cube", "brick"));
	AddObject(new GameObject("Ground", "cube", "wood", glm::vec3(0.0f, -15.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(100.0f, 5.0f, 100.0f)));
}

void UpdateScene(float timestep) {
	gScene.camera.get()->Update(timestep);
}