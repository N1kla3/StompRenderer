#pragma once

#include "glm/glm.hpp"

enum class ECameraMovement
{
    MOVE_FORWARD,
    MOVE_BACK,
    MOVE_RIGHT,
    MOVE_LEFT,
    MOVE_UP,
    MOVE_DOWN
};

struct InputData
{
    glm::vec3 input_vector{0.f};
    float yaw = 0.f;
    float pitch = 0.f;

    void reset()
    {
        input_vector = glm::vec3{0.f};
        yaw = 0.f;
        pitch = 0.f;
    }
};

namespace omp
{
    class Camera
    {
    private:
        static constexpr float YAW = 0.f;
        static constexpr float PITCH = 0.f;
        static constexpr float SPEED = 20.f;
        static constexpr float SENSITIVITY = 0.1f;
        static constexpr float ZOOM = 45.f;

    public:
        explicit Camera(
                glm::vec3 position = glm::vec3(0.f, 0.f, 0.f),
                glm::vec3 up = glm::vec3(0.f, 1.f, 0.f),
                float yaw = YAW,
                float pitch = PITCH);
        Camera(
                float posx,
                float posy,
                float posz,
                float upx,
                float upy,
                float upz,
                float yaw = YAW,
                float pitch = PITCH);

        virtual ~Camera() = default;

        glm::mat4 getViewMatrix() const;

        glm::vec3 getPosition() const { return m_Position; }

        void processKeyboard(ECameraMovement direction);
        void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
        [[maybe_unused]] void processMouseScroll(float yOffset);

        void applyInputs(float deltaTime);

        float getViewAngle() const { return m_ViewAngle; }

        float getNearClipping() const { return m_NearClipping; }

        float getFarClipping() const { return m_FarClipping; }

    private:
        glm::vec3 m_Position;
        glm::vec3 m_Front;
        glm::vec3 m_Up;
        glm::vec3 m_Right;
        glm::vec3 m_WorldUp;

        float m_Yaw;
        float m_Pitch;

        float m_MovementSpeed;
        float m_MouseSensitivity;
        float m_Zoom;

        InputData m_InputData;

        float m_ViewAngle = 60.f;
        float m_NearClipping = 0.1f;
        float m_FarClipping = 1000.f;

        void updateCameraVectors();

        friend class CameraPanel;
    };
}
