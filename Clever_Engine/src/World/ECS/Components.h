#pragma once
#include <glm.hpp>

#include "Objects/Vertex.h"


struct Camera
{
    glm::vec3 position{ 0.0f, 0.0f, 0.0f };
    glm::quat rotation = glm::identity<glm::quat>();
    Camera()
    {
        glm::quat qYaw = glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0));
        glm::quat qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));
        rotation = qYaw * qPitch;
    };
    float yaw = 0.0f;
    float pitch = 0.0f;
    float moveSpeed = 150.0f;
    float sensitivity = 0.4f;

    // Field of View in degrees (usually 45.0f to 90.0f)
    float fov = 90.0f;

    // Window Width / Window Height
    float aspectRatio = 1.777f; // Default 16:9

    // The closest distance the camera can see (don't use 0.0f!)
    float nearPlane = 0.1f;

    // The furthest distance the camera can see
    float farPlane = 1000.0f;

    // Optional: Perspective vs Orthographic toggle
    bool isPerspective = true;

    // Optional: Orthographic scale (used if isPerspective is false)
    float orthoSize = 10.0f;

    // Rotate the standard world axes by our rotation quaternion
    glm::vec3 GetForward() const {
        // Rotates (0,0,-1) by the quaternion
        return rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    }

    glm::vec3 GetUp() const {
        // Rotates (0,1,0) by the quaternion
        return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
    }

    glm::vec3 GetRight() const {
        // Rotates (1,0,0) by the quaternion
        return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    }
};