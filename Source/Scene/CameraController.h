#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

class Camera;

class CameraController {
public:
    void Update(Camera& camera, float timetep);
};

#endif 