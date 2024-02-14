#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <memory>

#include "Scene/GameObject.h"
#include "Scene/Camera.h"

struct Scene {
	std::vector<std::unique_ptr<GameObject>> objects;
	std::unique_ptr<Camera> camera;
};

void AddObject(GameObject* object);
void CreateScene();
void UpdateScene(float timestep);

extern Scene gScene;

#endif 