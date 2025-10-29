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
#include <optional>


//====================================================
//      NOTE: Application Structs
//====================================================
struct Application
{
    GLFWwindow *               m_window;

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
    VkCommandBuffer            m_commandBuffer;
    
    std::vector<VkImage>       m_swapChainImages;
    std::vector<VkImageView>   m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    VkDebugUtilsMessengerEXT   m_debugMessenger;

    // NOTE: Synchronization Object
    VkSemaphore                m_imageAvailableSemaphore;
    VkSemaphore                m_renderFinishedSemaphore;
    VkFence                    m_inFlightFence;
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
    VkSemaphore m_imageAvailableSemaphore;
    VkSemaphore m_renderFinishedSemaphore;
    VkFence     m_inFlightFence;
    
};

//====================================================
//      NOTE: Application Globals
//====================================================

constexpr int32 WIDTH = 800;
constexpr int32 HEIGHT = 600;

const char * validationLayers[] =
{
    "VK_LAYER_KHRONOS_validation"
};

const char * deviceExtensions[] =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME    
};

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

//====================================================
//      NOTE: Application Functions
//====================================================
void InitVulkan(Application & app);

void InitWindow(Application & app);

void MainLoop(Application & app);

void CleanUp(Application & app);

#define APPLICATION_H
#endif
