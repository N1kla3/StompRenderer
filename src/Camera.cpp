#include "Camera.h"
#include "glm/ext/matrix_transform.hpp"

omp::Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
        : m_Position(position)
        , m_Up(up)
        , m_WorldUp(up)
        , m_Yaw(yaw)
        , m_Pitch(pitch)
        , m_Front(glm::vec3(0.f, 0.f, -1.f))
        , m_MovementSpeed(SPEED)
        , m_MouseSensitivity(SENSITIVITY)
        , m_Zoom(ZOOM)
{
    updateCameraVectors();
}

omp::Camera::Camera(float posx, float posy, float posz, float upx, float upy, float upz, float yaw, float pitch)
        : m_Position(posx, posy, posz)
        , m_Up(upx, upy, upz)
        , m_WorldUp(upx, upy, upz)
        , m_Yaw(yaw)
        , m_Pitch(pitch)
        , m_Front(glm::vec3(0.f, 0.f, -1.f))
        , m_MovementSpeed(SPEED)
        , m_MouseSensitivity(SENSITIVITY)
        , m_Zoom(ZOOM)
{
    updateCameraVectors();
}

glm::mat4 omp::Camera::getViewMatrix() const
{
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

void omp::Camera::processKeyboard(ECameraMovement direction)
{
    if (direction == ECameraMovement::MOVE_FORWARD)
    {
        m_InputData.input_vector += m_Front;
    }
    if (direction == ECameraMovement::MOVE_BACK)
    {
        m_InputData.input_vector -= m_Front;
    }
    if (direction == ECameraMovement::MOVE_LEFT)
    {
        m_InputData.input_vector -= m_Right;
    }
    if (direction == ECameraMovement::MOVE_RIGHT)
    {
        m_InputData.input_vector += m_Right;
    }
    if (direction == ECameraMovement::MOVE_UP)
    {
        m_InputData.input_vector += m_Up;
    }
    if (direction == ECameraMovement::MOVE_DOWN)
    {
        m_InputData.input_vector -= m_Up;
    }
}

void omp::Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch)
{
    xOffset *= m_MouseSensitivity;
    yOffset *= m_MouseSensitivity;

    m_InputData.yaw += xOffset;
    m_InputData.pitch+= yOffset;

    if (constrainPitch)
    {
        if (m_Pitch > 89.f)
        {
            m_Pitch = 89.f;
        }
        if (m_Pitch < -89.f)
        {
            m_Pitch = -89.f;
        }
    }
}

void omp::Camera::processMouseScroll(float yOffset)
{

}

void omp::Camera::updateCameraVectors()
{
    glm::vec3 front(1.f);
    front.x = glm::cos(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
    front.y = glm::sin(glm::radians(m_Pitch));
    front.z = glm::sin(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);

    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}

void omp::Camera::applyInputs(float deltaTime)
{
    m_Yaw += m_InputData.yaw;
    m_Pitch += m_InputData.pitch;
    updateCameraVectors();
    if (m_InputData.input_vector != glm::vec3{0.f})
    {
        m_InputData.input_vector = glm::normalize(m_InputData.input_vector);
    }
    m_Position += m_InputData.input_vector * deltaTime * m_MovementSpeed;

    m_InputData.reset();
}
