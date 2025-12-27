#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

class Camera
{
public:
    glm::vec3 pos;
    glm::vec3 forward;
    glm::vec3 up;
    glm::vec3 worldUp;
    glm::vec3 right;
    
    float resistance = 0.8f;
    float _FOV = 90.0f;

    int sprintFrames = 0;

    Camera(glm::vec3 pos = glm::vec3(0.0f, 100.0f, 0.0f))
    {
        this->forward = glm::vec3(0.0f, 0.0f, 1.0f);
        this->worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        this->up = worldUp;

        pos = glm::vec3(0.0f);
        yaw = 90.0f;
        pitch = 0.0f;
        lastX = 1440.0f / 2.0f;
        lastY = 900.0f / 2.0f;
        firstMouse = true;

        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(pos, pos + forward, up);
    }

    glm::mat4 GetProjectionMatrix()
    {
        return glm::perspective(glm::radians(_FOV), 1440.0f / 900.0f, 0.001f, 1000.0f);
    }

    void setPos(glm::vec3 newPos)
    {
        pos = newPos;
    }

    void processInput(InputState state, float dt)
    {
        yaw += state.mouseX;
        pitch += state.mouseY;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        updateCameraVectors();
    }

    void updateFOV(bool sprinting)
    {
        if (sprinting)
        {
            sprintFrames++;
            sprintFrames = std::min(sprintFrames, sprintAnimDuration);
        }
        else
        {
            sprintFrames--;
            sprintFrames = std::max(sprintFrames, 0);
        }

        float t = (float)sprintFrames / (float)sprintAnimDuration;
        _FOV = FOV + (sprintFovAddition * t);
    }

private:
    float camSpeed = 2.0f;
    float mouseSensitivity = 10.0f;

    float moveSpeed = 5.0f;

    float FOV = 90.0f;
    int sprintAnimDuration = 10;
    int sprintFovAddition = 20;

    bool firstMouse = true;
    float lastX, lastY;
    float yaw, pitch;

    void updateCameraVectors()
    {
        glm::vec3 newForward;

        newForward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newForward.y = sin(glm::radians(pitch));
        newForward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        forward = glm::normalize(newForward);

        right = glm::normalize(glm::cross(forward, worldUp));
        up = glm::cross(right, forward);
    }
};