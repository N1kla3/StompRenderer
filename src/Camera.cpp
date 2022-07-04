#include "Camera.h"
#include "glm/ext/matrix_transform.hpp"

omp::Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
        : m_Position(position),
          m_Up(up),
          m_WorldUp(up),
          m_Yaw(yaw),
          m_Pitch(pitch),
          m_Front(glm::vec3(0.f, 0.f, -1.f)),
          m_MovementSpeed(SPEED),
          m_MouseSensitivity(SENSITIVITY),
          m_Zoom(ZOOM)
{
    UpdateCameraVectors();
}

omp::Camera::Camera(float posx, float posy, float posz, float upx, float upy, float upz, float yaw, float pitch)
        : m_Position(posx, posy, posz),
          m_Up(upx, upy, upz),
          m_WorldUp(upx, upy, upz),
          m_Yaw(yaw),
          m_Pitch(pitch),
          m_Front(glm::vec3(0.f, 0.f, -1.f)),
          m_MovementSpeed(SPEED),
          m_MouseSensitivity(SENSITIVITY),
          m_Zoom(ZOOM)
{
    UpdateCameraVectors();
}

glm::mat4 omp::Camera::GetViewMatrix() const
{
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

void omp::Camera::ProcessKeyboard(CAMERA_MOVEMENT direction, float deltaTime)
{
    float velocity = deltaTime * m_MovementSpeed;
    if (direction == CAMERA_MOVEMENT::MOVE_FORWARD)
        m_Position += m_Front * velocity;
    if (direction == CAMERA_MOVEMENT::MOVE_BACK)
        m_Position -= m_Front * velocity;
    if (direction == CAMERA_MOVEMENT::MOVE_LEFT)
        m_Position -= m_Right * velocity;
    if (direction == CAMERA_MOVEMENT::MOVE_RIGHT)
        m_Position += m_Right * velocity;
    if (direction == CAMERA_MOVEMENT::MOVE_UP)
        m_Position += m_Up * velocity;
    if (direction == CAMERA_MOVEMENT::MOVE_DOWN)
        m_Position -= m_Up * velocity;
}

void omp::Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch)
{
    xOffset *= m_MouseSensitivity;
    yOffset *= m_MouseSensitivity;

    m_Yaw += xOffset;
    m_Pitch += yOffset;

    if (constrainPitch)
    {
        if (m_Pitch > 89.f)
            m_Pitch = 89.f;
        if (m_Pitch < -89.f)
            m_Pitch = -89.f;
    }
    UpdateCameraVectors();
}

void omp::Camera::ProcessMouseScroll(float yOffset)
{

}

void omp::Camera::UpdateCameraVectors()
{
    glm::vec3 front(1.f);
    front.x = glm::cos(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
    front.y = glm::sin(glm::radians(m_Pitch));
    front.z = glm::sin(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);

    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}
