#if !defined(APPLICATION_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "engine_lib.h"

//====================================================
//      NOTE: Application Constexpr
//====================================================

/*

  NOTE: APP_SLOW
   1 - No optimization build (DEBUG build)
   0 - Build with optimization (Release build)
 */

#if APP_SLOW
constexpr bool enableValidationLayers = true;
#else
constexpr bool enableValidationLayers = false;
#endif

constexpr int32 WIDTH = 800;
constexpr int32 HEIGHT = 600;
constexpr int32 MAX_FRAMES_IN_FLIGHT = 2;

constexpr char * validationLayers[] =
{
    "VK_LAYER_KHRONOS_validation"
};

constexpr char * deviceExtensions[] =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME    
};


//====================================================
//      NOTE: Application Structs
//====================================================
struct Application
{

    uint32 m_currentFrame = 0;
    bool   m_framebufferResized = false;
    
    GLFWwindow *               m_window;

    VkDebugUtilsMessengerEXT   m_debugMessenger;

    VkInstance                 m_instance;
    VkPhysicalDevice           m_physicalDevice = VK_NULL_HANDLE;
    VkDevice                   m_device;
    VkQueue                    m_graphicsQueue;
    VkQueue                    m_presentQueue;
    VkSurfaceKHR               m_surface;
    VkSwapchainKHR             m_swapChain;
    VkFormat                   m_swapChainImageFormat;
    VkExtent2D                 m_swapChainExtent;
    VkRenderPass               m_renderPass;    
    VkPipelineLayout           m_pipelineLayout;
    VkPipeline                 m_graphicsPipline;
    VkCommandPool              m_commandPool;
    VkBuffer                   m_vertexBuffer;
    VkDeviceMemory             m_vertexBufferMemory;

    std::vector<VkImage>       m_swapChainImages;
    std::vector<VkImageView>   m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;
    Array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT>  m_commandBuffers;
    // NOTE: Synchronization Object
    Array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imageAvailableSemaphores;
    Array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_renderFinishedSemaphores;
    Array<VkFence, MAX_FRAMES_IN_FLIGHT>     m_inFlightFences;
};

struct IsDeviceSuitableResult
{
    bool m_isSuitable = false;
    int32 m_score = 0;
};

struct QueueFamilyIndices
{
    std::optional<uint32> m_graphicsFamily;
    std::optional<uint32> m_presentFamily;

    bool IsComplete()
    {
        return m_graphicsFamily.has_value() && m_presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR m_capabilities;
    std::vector<VkSurfaceFormatKHR> m_formats;
    std::vector<VkPresentModeKHR> m_presentMods;
};

struct CreateSwapChainResult
{
    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
};

struct CreateGraphicsPipelineResult
{
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipline;
};

struct SyncObjects
{
    Array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imageAvailableSemaphores;
    Array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_renderFinishedSemaphores;
    Array<VkFence, MAX_FRAMES_IN_FLIGHT>     m_inFlightFences;
};

struct BufferCreateResult
{
    VkBuffer       m_buffer;
    VkDeviceMemory m_bufferMemory;
};

struct Vertex
{
    glm::vec2 m_pos;
    glm::vec3 m_color;
};

//====================================================
//      NOTE: Application Globals
//====================================================
constexpr Vertex vertices[] =
{
    { { 0.0f, -0.5f }, { 1.0f, 1.0f, 1.0f } },
    { { 0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } },
    { {-0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } }
};


//====================================================
//      NOTE: Application Functions
//====================================================
void RunApplication(Application & app);

#define APPLICATION_H
#endif
