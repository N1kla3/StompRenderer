#include "Camera.h"
#include "SceneEntity.h"
#include "glm/ext/matrix_transform.hpp"

omp::Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
        : SceneEntity()
        , m_Position(position)
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
        : SceneEntity()
        , m_Position(posx, posy, posz)
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

void omp::Camera::processMouseScroll(float /*yOffset*/)
{

}

void omp::Camera::onSceneSave(omp::JsonParser<>& parser, omp::Scene* scene)
{
    SceneEntity::onSceneSave(parser, scene);

    parser.writeValue("camerapos_x", m_Position.x);
    parser.writeValue("camerapos_y", m_Position.y);
    parser.writeValue("camerapos_z", m_Position.z);

    parser.writeValue("front_x", m_Front.x);
    parser.writeValue("front_y", m_Front.y);
    parser.writeValue("front_z", m_Front.z);

    parser.writeValue("up_x", m_Up.x);
    parser.writeValue("up_y", m_Up.y);
    parser.writeValue("up_z", m_Up.z);

    parser.writeValue("right_x", m_Right.x);
    parser.writeValue("right_y", m_Right.y);
    parser.writeValue("right_z", m_Right.z);

    parser.writeValue("worldup_x", m_WorldUp.x);
    parser.writeValue("worldup_y", m_WorldUp.y);
    parser.writeValue("worldup_z", m_WorldUp.z);

    parser.writeValue("yaw", m_Yaw);
    parser.writeValue("pitch", m_Pitch);

    parser.writeValue("view_angle", m_ViewAngle);
    parser.writeValue("near_clipping",  m_NearClipping);
    parser.writeValue("far_clipping", m_FarClipping);
}

void omp::Camera::onSceneLoad(omp::JsonParser<>& parser, omp::Scene* scene)
{
    SceneEntity::onSceneLoad(parser, scene);

    m_Position.x = parser.readValue<float>("camerapos_x").value();
    m_Position.y = parser.readValue<float>("camerapos_y").value();
    m_Position.z = parser.readValue<float>("camerapos_z").value();

    m_Front.x = parser.readValue<float>("front_x").value();
    m_Front.y = parser.readValue<float>("front_y").value();
    m_Front.z = parser.readValue<float>("front_z").value();

    m_Up.x = parser.readValue<float>("up_x").value();
    m_Up.y = parser.readValue<float>("up_y").value();
    m_Up.z = parser.readValue<float>("up_z").value();

    m_Right.x = parser.readValue<float>("right_x").value();
    m_Right.y = parser.readValue<float>("right_y").value();
    m_Right.z = parser.readValue<float>("right_z").value();

    m_WorldUp.x = parser.readValue<float>("worldup_x").value();
    m_WorldUp.y = parser.readValue<float>("worldup_y").value();
    m_WorldUp.z = parser.readValue<float>("worldup_z").value();

    m_Yaw = parser.readValue<float>("yaw").value();
    m_Pitch = parser.readValue<float>("pitch").value();

    m_ViewAngle = parser.readValue<float>("view_angle").value();
    m_NearClipping = parser.readValue<float>("near_clipping").value();
    m_FarClipping = parser.readValue<float>("far_clipping").value();
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
