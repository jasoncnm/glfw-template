/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
#include "application.h"

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


internal void InitWindow(Application & app)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    app.m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    glfwSetWindowUserPointer(app.m_window, &app);
    glfwSetFramebufferSizeCallback(app.m_window, FramebufferResizeCallback);
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
        ImguiStartFrame(app);

        // Rendering
        ImGui::Render();
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

