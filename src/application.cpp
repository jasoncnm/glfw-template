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

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

/*

TODO: Things that I can do
  - Basic perspective camera control
  - Impliment custom hash map without using std::hash and std::unordered_map
  - Impliment custom arena allocator for vulkan object allocations
  - Continue mipmaping tutorial
  - Draw shader arts 
*/


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

    
    {
        DepthResourcesCreateResult result = CreateDepthResources(app.m_device, app.m_physicalDevice, app.m_swapChainExtent);

        app.m_depthImage       = result.m_depthImageResult.m_image;
        app.m_depthImageMemory = result.m_depthImageResult.m_imageMemory;
        app.m_depthImageView   = result.m_depthImageView;
    }

    
    app.m_swapChainFramebuffers = CreateFramebuffers(app.m_device, app.m_swapChainImageViews, app.m_depthImageView, app.m_renderPass, app.m_swapChainExtent);
}

internal void UpdateUniformBuffer(Application & app)
{
    local_persist auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    real32 timeElapsed = std::chrono::duration<real32, std::chrono::seconds::period>(currentTime - startTime).count();
    timeElapsed = 0.0f;
    
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
    VkClearValue clearDepthStencil = { 1.0f, 0 };

    VkClearValue clearValues[] = { clearColor, clearDepthStencil };
    renderPassInfo.clearValueCount = ArrayCount(clearValues);
    renderPassInfo.pClearValues = clearValues;

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

    vkCmdBindIndexBuffer(commandBuffer, app.m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            app.m_pipelineLayout,
                            0,
                            1,
                            &app.m_descriptorSets[app.m_currentFrame],
                            0,
                            nullptr);

    vkCmdDrawIndexed(commandBuffer, (uint32)app.m_model.m_indices.size(), 1, 0, 0, 0);

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

internal Model LoadModel()
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    bool ret = !tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH);
    if (!warn.empty())
    {
        SM_WARN("%s", warn.c_str());
    }
    if (!err.empty())
    {
        SM_ASSERT(false, "failed to load object file, err: %s", err.c_str());
        // SM_ERROR("%s", err.c_str());
    }

    std::unordered_map<Vertex, uint32> uniqueVertices = {};
    Model model = {};

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++)
    {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++)
            {
                Vertex vertex = {};

                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                vertex.m_pos.x = attrib.vertices[3*size_t(idx.vertex_index)+0];
                vertex.m_pos.y = attrib.vertices[3*size_t(idx.vertex_index)+1];
                vertex.m_pos.z = attrib.vertices[3*size_t(idx.vertex_index)+2];

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
                    tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
                    tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    vertex.m_texCoord.x = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
                    vertex.m_texCoord.y = 1.0f - attrib.texcoords[2*size_t(idx.texcoord_index)+1];
                }

                vertex.m_color = { 1.0f, 1.0f, 1.0f };
            
                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = (uint32)model.m_vertices.size();
                    model.m_vertices.push_back(vertex);
                }
                
                model.m_indices.push_back(uniqueVertices[vertex]);
            }
            
            index_offset += fv;
            // per-face material
            // shapes[s].mesh.material_ids[f];
        }
    }

    size_t vertexSize = model.m_vertices.size();
    
    return model;
}

internal void InitVulkan(Application & app)
{
    app.m_model = LoadModel();
    
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
    app.m_renderPass          = CreateRenderPass(app.m_device, app.m_physicalDevice, app.m_swapChainImageFormat);
    app.m_descriptorSetLayout = CreateDescriptorSetLayout(app.m_device);    

    {
        CreateGraphicsPipelineResult result =
            CreateGraphicsPipeline(app.m_device, app.m_swapChainExtent, app.m_renderPass, app.m_descriptorSetLayout);

        app.m_pipelineLayout  = result.m_pipelineLayout;
        app.m_graphicsPipline = result.m_graphicsPipline;
    }
    
    app.m_commandPool = CreateCommandPool(app.m_device, app.m_physicalDevice, app.m_surface);
    
    {
        ImageCreateResult result = CreateTextureImage(app.m_device, app.m_physicalDevice, app.m_commandPool, app.m_graphicsQueue);
        app.m_textureImage       = result.m_image;
        app.m_textureImageMemory = result.m_imageMemory;
    }
    
    app.m_textureImageView = CreateTextureImageView(app.m_device, app.m_textureImage);
    app.m_textureSampler   = CreateTextureSampler(app.m_device, app.m_physicalDevice);

    {
        DepthResourcesCreateResult result = CreateDepthResources(app.m_device, app.m_physicalDevice, app.m_swapChainExtent);

        app.m_depthImage       = result.m_depthImageResult.m_image;
        app.m_depthImageMemory = result.m_depthImageResult.m_imageMemory;
        app.m_depthImageView   = result.m_depthImageView;
    }

    app.m_swapChainFramebuffers = CreateFramebuffers(app.m_device, app.m_swapChainImageViews, app.m_depthImageView, app.m_renderPass, app.m_swapChainExtent);
    
    {
        BufferCreateResult result = CreateAndBindVertexBuffer(app.m_device, app.m_commandPool, app.m_graphicsQueue, app.m_physicalDevice, app.m_model.m_vertices);
        app.m_vertexBuffer        = result.m_buffer;
        app.m_vertexBufferMemory  = result.m_bufferMemory;
    }

    {
        BufferCreateResult result = CreateAndBindIndexBuffer(app.m_device, app.m_commandPool, app.m_graphicsQueue, app.m_physicalDevice, app.m_model.m_indices);
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
    app.m_descriptorSets = CreateDescriptorSets(app.m_device,
                                                app.m_uniformBuffers,
                                                app.m_descriptorPool,
                                                app.m_descriptorSetLayout,
                                                app.m_textureImageView,
                                                app.m_textureSampler);
    
    app.m_commandBuffers = CreateCommandBuffers(app.m_device, app.m_commandPool);

    {
        SyncObjects syncObjs           = CreateSyncObjects(app.m_device);
        app.m_imageAvailableSemaphores = syncObjs.m_imageAvailableSemaphores;
        app.m_renderFinishedSemaphores = syncObjs.m_renderFinishedSemaphores;
        app.m_inFlightFences           = syncObjs.m_inFlightFences;
    }
    
}
    

internal void InitWindow(Application & app)
{
    SM_ASSERT(glfwInit(), "Could not initilize GLFW!");
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    app.m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    SM_ASSERT(app.m_window, "Could not create window");
    SM_ASSERT(glfwVulkanSupported(), "GLFW: Vulkan Not Supported\n");

    app.m_clearColor = glm::vec4(HexToRGB(0x142A9C), 1.0f);

    
    GLFWmonitor * primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode * videoMode = glfwGetVideoMode(primaryMonitor);
    int32 windowLeft = videoMode->width / 2 - WIDTH / 2;
    int32 windowTop = videoMode->height / 2 - HEIGHT / 2;
    glfwSetWindowPos(app.m_window, windowLeft, windowTop);

    glfwSetWindowUserPointer(app.m_window, &app);
    glfwSetFramebufferSizeCallback(app.m_window, FramebufferResizeCallback);

    // NOTE: SetVSync;
    glfwSwapInterval(1);
    app.m_vSync = true;

    // NOTE: Set MouseInputOptions
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
                if (keyname)
                {
                    SM_TRACE("presed key: %s", keyname);
                }
                else
                {
                    SM_TRACE("presed keycode: %d", key); 
                }
                
                break;
            }
            case GLFW_RELEASE:
            {

                const char * keyname = glfwGetKeyName(key, scancode);
                if (keyname)
                {
                    SM_TRACE("released key: %s", keyname);
                }
                else
                {
                    SM_TRACE("released keycode: %d", key); 
                }
                
                break;
            }
            case GLFW_REPEAT:
            {

                const char * keyname = glfwGetKeyName(key, scancode);
                if (keyname)
                {
                    SM_TRACE("repeated key: %s", keyname);
                }
                else
                {
                    SM_TRACE("repeated keycode: %d", key); 
                }
                
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
        // SM_TRACE("mouse position: (%.2f, %.2f)", xpos, ypos);
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

    vkDestroySampler(app.m_device, app.m_textureSampler, nullptr);
    vkDestroyImageView(app.m_device, app.m_textureImageView, nullptr);
    vkDestroyImage(app.m_device, app.m_textureImage, nullptr);
    vkFreeMemory(app.m_device, app.m_textureImageMemory, nullptr);

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

