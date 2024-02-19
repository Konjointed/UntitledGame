#include "Camera.h"

#include "Event/EventManager.h"
#include "Scene/CameraController.h"

Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
    : mFOV(fov), mAspectRatio(aspectRatio), mNearPlane(nearPlane), mFarPlane(farPlane),
    mPosition(0.0f, 0.0f, 3.0f), mForward(0.0f, 0.0f, -1.0f), mUp(0.0f, 1.0f, 0.0f),
    mYaw(0.0f), mPitch(0.0f) 
{
    gEventManager.Connect<WindowResizeEvent>(this, &Camera::onWindowResize);
}

void Camera::Update(float timestep)
{
    mController->Update(*this, timestep);
}

void Camera::Pivot(float deltaX, float deltaY, float sensitivity)
{
    mYaw += deltaX * sensitivity;
    mPitch -= deltaY * sensitivity;
    mPitch = glm::clamp(mPitch, -89.0f, 89.0f);

    updateCameraVectors();
}

void Camera::MoveForward(float distance)
{
    mPosition += mForward * distance;
}

void Camera::MoveBackward(float distance)
{
    mPosition -= mForward * distance;
}

void Camera::StrafeLeft(float distance)
{
    glm::vec3 mRight = glm::normalize(glm::cross(mForward, mUp));
    mPosition -= mRight * distance;
}

void Camera::StrafeRight(float distance)
{
    glm::vec3 mRight = glm::normalize(glm::cross(mForward, mUp));
    mPosition += mRight * distance;
}

void Camera::SetController(CameraController* controller)
{
    mController = controller;
}

glm::mat4 Camera::GetProjection() const {
    return glm::perspective(glm::radians(mFOV), mAspectRatio, mNearPlane, mFarPlane);
}

glm::mat4 Camera::GetView() const {
    return glm::lookAt(mPosition, mPosition + mForward, mUp);
}

glm::vec3 Camera::GetPosition() const {
    return mPosition;
}

float Camera::GetNearPlane() const {
    return mNearPlane;
}

float Camera::GetFarPlane() const {
    return mFarPlane;
}

float Camera::GetAspectRatio() const
{
    return mAspectRatio;
}

void Camera::onWindowResize(const WindowResizeEvent& event)
{
    mAspectRatio = static_cast<float>(event.x) / static_cast<float>(event.y);
}

void Camera::updateCameraVectors()
{
    glm::vec3 forward;
    forward.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    forward.y = sin(glm::radians(mPitch));
    forward.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    mForward = glm::normalize(forward);

    mUp = glm::vec3(0.0f, 1.0f, 0.0f);
}
