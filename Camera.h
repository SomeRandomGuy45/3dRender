#include "helper.h"


// Camera class
class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
        : Position(position), WorldUp(up), Yaw(yaw), Pitch(pitch) {
        updateCameraVectors();
    }

    void setProjection(float aspectRatio) {
        Projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    glm::mat4 getProjectionMatrix() const {
        return Projection;
    }

    void processKeyboard(float deltaTime) {
        const float cameraSpeed = 2.5f * deltaTime;
        glm::vec3 resetC = Front;
        glm::vec3 resetUP = Up;
        Up.x = 0.0f;
        Up.z = 0.0f;
        Front.y = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            Position += Front * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            Position -= Front * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            Position -= Right * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            Position += Right * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            Position += Up * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
            Position -= Up * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            auto CURSOR_TYPE = locked ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
            glfwSetInputMode(window, GLFW_CURSOR, CURSOR_TYPE);
            locked = locked ? false : true;
        }
        Front = resetC;
        Up = resetUP;
    }

    void processMouseMovement(float xoffset, float yoffset) {
        const float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        Yaw += xoffset;
        Pitch -= yoffset;

        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;

        updateCameraVectors();
    }

private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }

    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    glm::mat4 Projection;

    float Yaw;
    float Pitch;
};