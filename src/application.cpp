/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
#include "application.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//====================================================
//      NOTE: Application Functions
//====================================================
#include "imgui_setup.cpp"
#include "vulkan_setup.cpp"

internal void RecreateSwapChain(Application & app)
{
    int32 width = 0, height = 0;
    glfwGetFramebufferSize(app.m_window, &width, &height);
    for ( ; width == 0 || height == 0; )
    {
        glfwGetFramebufferSize(app.m_window, &width, &height);
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(app.m_device);
    CleanupSwapChain(app);
    
    // NOTE: swapchain, images, format, extent creation
    {
        CreateSwapChainResult createResult = CreateSwapChain(app.m_window, app.m_device, app.m_physicalDevice, app.m_surface);
        app.m_swapChain = createResult.m_swapChain;
        app.m_swapChainImages = createResult.m_swapChainImages;
        app.m_swapChainImageFormat = createResult.m_swapChainImageFormat;
        app.m_swapChainExtent = createResult.m_swapChainExtent;
    }
    app.m_swapChainImageViews = CreateImageViews(app.m_swapChainImages, app.m_device, app.m_swapChainImageFormat);
    app.m_swapChainFramebuffers = CreateFramebuffers(app.m_device, app.m_swapChainImageViews, app.m_renderPass, app.m_swapChainExtent);
}

internal void UpdateUniformBuffer(Application & app)
{
    local_persist auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    real32 timeElapsed = std::chrono::duration<real32, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo = {};
    ubo.m_model      = glm::rotate(glm::mat4(1.0f), timeElapsed * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.m_view       = glm::lookAt(glm::vec3(2.0f, 2.0f, app.m_zoom), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.m_projection = glm::perspective(glm::radians(45.0f), app.m_swapChainExtent.width / (real32) app.m_swapChainExtent.height, 0.1f, 10.0f);
    ubo.m_projection[1][1] *= -1;

    memcpy(app.m_uniformBuffersMapped[app.m_currentFrame], &ubo, sizeof(ubo));
    
}

internal
void RecordCommandBuffer(Application & app, uint32 imageIndex)
{
    VkCommandBuffer & commandBuffer = app.m_commandBuffers[app.m_currentFrame];
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to begine recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = app.m_renderPass;
    renderPassInfo.framebuffer = app.m_swapChainFramebuffers[imageIndex];    
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = app.m_swapChainExtent;

    VkClearValue clearColor =  { { app.m_clearColor.r, app.m_clearColor.g, app.m_clearColor.b, app.m_clearColor.a } };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, app.m_graphicsPipline);

    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (real32)app.m_swapChainExtent.width;
    viewport.height = (real32)app.m_swapChainExtent.height;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = app.m_swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = { app.m_vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, app.m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            app.m_pipelineLayout,
                            0,
                            1,
                            &app.m_descriptorSets[app.m_currentFrame],
                            0,
                            nullptr);
    
    vkCmdDrawIndexed(commandBuffer, (uint32)ArrayCount(vertexIndices), 1, 0, 0, 0);

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to record command buffer!");
    }
}

internal void DrawFrame(Application & app)
{

    vkWaitForFences(app.m_device, 1, &app.m_inFlightFences[app.m_currentFrame], VK_TRUE, UINT64_MAX);

    uint32 imageIndex;
    VkResult result = vkAcquireNextImageKHR(app.m_device,
                                            app.m_swapChain,
                                            UINT64_MAX,
                                            app.m_imageAvailableSemaphores[app.m_currentFrame],
                                            VK_NULL_HANDLE,
                                            &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain(app);
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        SM_ASSERT(false, "failed to acquire swap chain image!");
    }

    vkResetFences(app.m_device, 1, &app.m_inFlightFences[app.m_currentFrame]);
    
    vkResetCommandBuffer(app.m_commandBuffers[app.m_currentFrame], 0);
    RecordCommandBuffer(app, imageIndex);

    UpdateUniformBuffer(app);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { app.m_imageAvailableSemaphores[app.m_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = ArrayCount(waitSemaphores);
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &app.m_commandBuffers[app.m_currentFrame];

    VkSemaphore signalSemaphores[] = { app.m_renderFinishedSemaphores[app.m_currentFrame] };
    submitInfo.signalSemaphoreCount = ArrayCount(signalSemaphores);
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(app.m_graphicsQueue, 1, &submitInfo, app.m_inFlightFences[app.m_currentFrame]) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to submit draw command buffer!");
    }

    
    // Update and Render additional Platform Windows
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }


    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = ArrayCount(signalSemaphores);
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { app.m_swapChain };
    presentInfo.swapchainCount = ArrayCount(swapChains);
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    

    result = vkQueuePresentKHR(app.m_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || app.m_framebufferResized)
    {
        app.m_framebufferResized = false;
        RecreateSwapChain(app);
    }
    else if (result != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to present swap chain image!");
    }

    app.m_currentFrame = (app.m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}


internal void InitVulkan(Application & app)
{
    app.m_instance       = CreateVkInstance();
    app.m_debugMessenger = SetupDebugMessenger(app.m_instance);
    app.m_surface        = CreateSurface(app.m_window, app.m_instance);
    app.m_physicalDevice = PickPhysicalDevice(app.m_instance, app.m_surface);
    app.m_device         = CreateLogicalDevice(app.m_physicalDevice, app.m_surface);
    app.m_graphicsQueue  = CreateGraphicsQueue(app.m_device, app.m_physicalDevice, app.m_surface);
    app.m_presentQueue   = CreatePresentQueue(app.m_device, app.m_physicalDevice, app.m_surface);

    // NOTE: swapchain, images, format, extent creation
    {
        CreateSwapChainResult createResult = CreateSwapChain(app.m_window, app.m_device, app.m_physicalDevice, app.m_surface);

        app.m_swapChain            = createResult.m_swapChain;
        app.m_swapChainImages      = createResult.m_swapChainImages;
        app.m_swapChainImageFormat = createResult.m_swapChainImageFormat;
        app.m_swapChainExtent      = createResult.m_swapChainExtent;
    }

    app.m_swapChainImageViews = CreateImageViews(app.m_swapChainImages, app.m_device, app.m_swapChainImageFormat);
    app.m_renderPass          = CreateRenderPass(app.m_device, app.m_swapChainImageFormat);
    app.m_descriptorSetLayout = CreateDescriptiorSetLayout(app.m_device);    

    {
        CreateGraphicsPipelineResult result =
            CreateGraphicsPipeline(app.m_device, app.m_swapChainExtent, app.m_renderPass, app.m_descriptorSetLayout);

        app.m_pipelineLayout  = result.m_pipelineLayout;
        app.m_graphicsPipline = result.m_graphicsPipline;
    }
    
    app.m_swapChainFramebuffers = CreateFramebuffers(app.m_device, app.m_swapChainImageViews, app.m_renderPass, app.m_swapChainExtent);
    app.m_commandPool           = CreateCommandPool(app.m_device, app.m_physicalDevice, app.m_surface);

    {
        BufferCreateResult result = CreateAndBindVertexBuffer(app.m_device, app.m_commandPool, app.m_graphicsQueue, app.m_physicalDevice);
        app.m_vertexBuffer        = result.m_buffer;
        app.m_vertexBufferMemory  = result.m_bufferMemory;
    }

    {
        BufferCreateResult result = CreateAndBindIndexBuffer(app.m_device, app.m_commandPool, app.m_graphicsQueue, app.m_physicalDevice);
        app.m_indexBuffer         = result.m_buffer;
        app.m_indexBufferMemory   = result.m_bufferMemory;
    }

    {
        UniformBufferCreateResult result = CreateUniformBuffers(app.m_device, app.m_physicalDevice);
        app.m_uniformBuffers       = result.m_uniformBuffers;
        app.m_uniformBuffersMemory = result.m_uniformBuffersMemory;
        app.m_uniformBuffersMapped = result.m_uniformBuffersMapped;
    }

    app.m_descriptorPool = CreateDescriptorPool(app.m_device);
    app.m_descriptorSets = CreateDescriptorSets(app.m_device, app.m_uniformBuffers, app.m_descriptorPool, app.m_descriptorSetLayout);
    
    app.m_commandBuffers = CreateCommandBuffers(app.m_device, app.m_commandPool);

    {
        SyncObjects syncObjs           = CreateSyncObjects(app.m_device);
        app.m_imageAvailableSemaphores = syncObjs.m_imageAvailableSemaphores;
        app.m_renderFinishedSemaphores = syncObjs.m_renderFinishedSemaphores;
        app.m_inFlightFences           = syncObjs.m_inFlightFences;
    }
    
}

internal void SetVSync(Application & app, bool enabled)
{
    if (enabled) glfwSwapInterval(1);
    else glfwSwapInterval(0);

    app.m_vSync = enabled;
}

internal void InitWindow(Application & app)
{
    SM_ASSERT(glfwInit(), "Could not initilize GLFW!");
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    app.m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    SM_ASSERT(app.m_window, "Could not create window");
    SM_ASSERT(glfwVulkanSupported(), "GLFW: Vulkan Not Supported\n");

    
    GLFWmonitor * primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode * videoMode = glfwGetVideoMode(primaryMonitor);
    int32 windowLeft = videoMode->width / 2 - WIDTH / 2;
    int32 windowTop = videoMode->height / 2 - HEIGHT / 2;
    glfwSetWindowPos(app.m_window, windowLeft, windowTop);

    glfwSetWindowUserPointer(app.m_window, &app);
    glfwSetFramebufferSizeCallback(app.m_window, FramebufferResizeCallback);
    SetVSync(app, true);

    app.m_clearColor = glm::vec4(HexToRGB(0x2B7CB4), 1.0f);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(app.m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    
    // NOTE: Set GLFW callbacks
    glfwSetKeyCallback(app.m_window, [](GLFWwindow * _window, int32 key, int32 scancode, int32 action, int32 mods)
    {
        // Application * app = (Application *)glfwGetWindowUserPointer(_window);

        switch(action)
        {
            case GLFW_PRESS:
            {
                const char * keyname = glfwGetKeyName(key, scancode);
                SM_TRACE("presed key: %s", keyname); 
                
                break;
            }
            case GLFW_RELEASE:
            {

                const char * keyname = glfwGetKeyName(key, scancode);
                SM_TRACE("released key: %s", keyname); 

                break;
            }
            case GLFW_REPEAT:
            {

                const char * keyname = glfwGetKeyName(key, scancode);
                SM_TRACE("repeated key: %s", keyname); 

                break;
            }
            default: {}
        }

    });


    glfwSetMouseButtonCallback(app.m_window, [](GLFWwindow * _window, int32 button, int32 action, int32 mods)
    {
        switch(action)
        {
            case GLFW_PRESS:
            {
                SM_TRACE("presed mouse button: %d", button);                
                break;
            }
            case GLFW_RELEASE:
            {
                SM_TRACE("released mouse button: %d", button);                
                break;
            }
            default: {}
        }
    });

    glfwSetScrollCallback(app.m_window, [](GLFWwindow* window, double xoffset, double yoffset)
    {
        SM_TRACE("MouseScroll: (%.1f, %.1f)", xoffset, yoffset);
    });

    glfwSetCursorPosCallback(app.m_window, [](GLFWwindow* window, double xpos, double ypos)
    {
        SM_TRACE("mouse position: (%.2f, %.2f)", xpos, ypos);
    });

    glfwSetJoystickCallback ([](int jid, int event)
    {
        if (event == GLFW_CONNECTED)
        {
            // The joystick was connected
            const char* name = glfwGetJoystickName(jid);
            SM_TRACE("joystick %s is connected, joystick id: %d", name, jid);
        }
        else if (event == GLFW_DISCONNECTED)
        {
            // The joystick was disconnected
            const char* name = glfwGetJoystickName(jid);
            SM_TRACE("joystick %s is disconnected, joystick id: %d", name, jid);
        }
    });
    
}

internal void CleanUp(Application & app)
{
    CleanupImgui();
    CleanupSwapChain(app);

    for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(app.m_device, app.m_uniformBuffers[i], nullptr);
        vkFreeMemory(app.m_device, app.m_uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(app.m_device, app.m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(app.m_device, app.m_descriptorSetLayout, nullptr);
    vkDestroyBuffer(app.m_device, app.m_vertexBuffer, nullptr);
    vkFreeMemory(app.m_device, app.m_vertexBufferMemory, nullptr);
    vkDestroyBuffer(app.m_device, app.m_indexBuffer, nullptr);
    vkFreeMemory(app.m_device, app.m_indexBufferMemory, nullptr);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(app.m_device, app.m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(app.m_device, app.m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(app.m_device, app.m_inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(app.m_device, app.m_commandPool, nullptr);
    vkDestroyPipeline(app.m_device, app.m_graphicsPipline, nullptr);
    vkDestroyPipelineLayout(app.m_device, app.m_pipelineLayout, nullptr);
    vkDestroyRenderPass(app.m_device, app.m_renderPass, nullptr);
    vkDestroyDevice(app.m_device, nullptr);
    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(app.m_instance, app.m_debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(app.m_instance, app.m_surface, nullptr);
    vkDestroyInstance(app.m_instance, nullptr);

    glfwDestroyWindow(app.m_window);
    glfwTerminate();
}


// NOTE: call at each frame
internal void MainLoop(Application & app)
{
    for ( ;!glfwWindowShouldClose(app.m_window); )
    {
        glfwPollEvents();
        //PrintAvailableJoyStics();
        ImguiStartFrame(app);

        // Rendering
        DrawFrame(app);
    }

    vkDeviceWaitIdle(app.m_device);
}


void RunApplication(Application & app)
{
    // NOTE: Run Application
    InitWindow(app);
    InitVulkan(app);
    InitImGui(app);
    MainLoop(app);
    CleanUp(app);
}

