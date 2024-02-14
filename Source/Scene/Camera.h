#ifndef CAMERA_H
#define CAMERA_H

#include "Core/Math.h"

class CameraController;

class Camera {
public:
    Camera(float fov = 70.0f, float aspectRatio = 1.777778f, float nearPlane = 0.1f, float farPlane = 500.0f);
    ~Camera() = default;

    void Update(float timestep);

    void Pivot(float deltaX, float deltaY, float sensitivity);
    void MoveForward(float distance);
    void MoveBackward(float distance);
    void StrafeLeft(float distance);
    void StrafeRight(float distance);

    void SetController(CameraController* controller);

    glm::mat4 GetProjection() const;
    glm::mat4 GetView() const;
    glm::vec3 GetPosition() const;
    float GetNearPlane() const;
    float GetFarPlane() const;
private:
    void updateCameraVectors();

    glm::vec3 mPosition;
    glm::vec3 mForward;
    glm::vec3 mUp;

    float mYaw;
    float mPitch;
    //float mRoll;

    float mFOV;
    float mAspectRatio;
    float mNearPlane;
    float mFarPlane;

    CameraController* mController;
};

#endif 