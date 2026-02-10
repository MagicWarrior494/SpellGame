#pragma once
#include <cstdint>
#include <iostream>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp> // Required for glm::lookAt and glm::perspective

#include "World/WorldController.h"
#include "Event/EventController.h"


#include "World/ECS/Registry.h"
#include "World/ECS/Components.h"
#include "Render/Window/WindowControls.h"

struct SceneCreationInfo {
    uint8_t windowID;
	uint8_t sceneID;
    uint32_t width;
    uint32_t height;
    int posx;
    int posy;
    int zIndex = 1; // Default Z-Index for input priority
};

class Scene : public IInputLayer
{
    void Update()
    {

        //gpuData.view = glm::lookAt(
        //    camera.position,
        //    camera.position + camera.GetForward(),
        //    glm::vec3(0.0f, 1.0f, 0.0f) // World Up
        //);

        //gpuData.proj = glm::perspective(
        //    glm::radians(camera.fov),
        //    camera.aspectRatio,
        //    camera.nearPlane,
        //    camera.farPlane
        //);

        //gpuData.proj[1][1] *= -1;

        //auto* manager = SharedCameraSceneData::GetManager();
        //if (manager)
        //{
        //    manager->UpdateSlot(m_CameraSlot, &gpuData);
        //}
    }

    // Example of handling specific input in a scene
    virtual void OnInput(InputEvent& event) override
    {
        if (event.type == InputEvent::Type::Key &&
            (event.action == Input::Action::PRESS || event.action == Input::Action::REPEAT))
        {
            //auto& camera = m_Registry.GetComponent<Camera>(m_CameraEntityID);

            //glm::vec3 forward = camera.GetForward();
            //glm::vec3 right = camera.GetRight();

            //float speed = camera.moveSpeed * m_VulkanContext->GetDeltaTime();

            ////std::cout << speed << " " << m_VulkanContext->GetDeltaTime() << std::endl;

            //if (event.code == Input::Keyboard::KEY_W) camera.position += forward * speed;
            //if (event.code == Input::Keyboard::KEY_S) camera.position -= forward * speed;
            //if (event.code == Input::Keyboard::KEY_A) camera.position -= right * speed;
            //if (event.code == Input::Keyboard::KEY_D) camera.position += right * speed;

            //event.Consume();
        }
        if (event.type == InputEvent::Type::MouseMove)
        {
            //auto& camera = m_Registry.GetComponent<Camera>(m_CameraEntityID);

            //// 1. Update Euler angles based on mouse movement
            //// deltaX: positive means mouse moved right
            //// deltaY: positive means mouse moved down
            //camera.yaw -= event.deltaX * camera.sensitivity;
            //camera.pitch -= event.deltaY * camera.sensitivity;

            //// 2. Clamp the pitch to prevent the "Backflip" bug
            //// Looking straight up or down causes the View Matrix to collapse (Gimbal Lock)
            //camera.pitch = glm::clamp(camera.pitch, -89.0f, 89.0f);

            //// 3. Rebuild the Rotation Quaternion
            //// We use the same order as your successful A/D test
            //glm::quat qYaw = glm::angleAxis(glm::radians(camera.yaw), glm::vec3(0, 1, 0));
            //glm::quat qPitch = glm::angleAxis(glm::radians(camera.pitch), glm::vec3(1, 0, 0));

            //camera.rotation = qYaw * qPitch;
            //camera.rotation = glm::normalize(camera.rotation);

            //event.Consume();
        }
    }
};