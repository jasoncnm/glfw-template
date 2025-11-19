#if !defined(APPLICATION_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "engine_lib.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>

#include <chrono>
#include <unordered_map>

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

constexpr int32 WIDTH = 1920;
constexpr int32 HEIGHT = 1080;
constexpr int32 MAX_FRAMES_IN_FLIGHT = 2;

constexpr char * validationLayers[] =
{
    "VK_LAYER_KHRONOS_validation"
};

constexpr char * deviceExtensions[] =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME    
};

constexpr char * TEXTURE_PATH = "resources/objects/viking_room/viking_room.png";
constexpr char * MODEL_PATH   = "resources/objects/viking_room/viking_room.obj";

//====================================================
//      NOTE: Application Structs
//====================================================

struct Vertex
{
    glm::vec3 m_pos;
    glm::vec3 m_color;
    glm::vec2 m_texCoord;

    bool operator==(const Vertex & other) const
    {
        return m_pos == other.m_pos && m_color == other.m_color && m_texCoord == other.m_texCoord;
    }
};

struct UniformBufferObject
{

/*
==========================================================================================================
                                                 IMPORTANT
==========================================================================================================
  Vulkan expects the data in your structure to be aligned in memory in a specific way, for example:
  - Scalars have to be aligned by N (= 4 bytes given 32 bit floats).
  - A vec2 must be aligned by 2N (= 8 bytes)
  - A vec3 or vec4 must be aligned by 4N (= 16 bytes)
  - A nested structure must be aligned by the base alignment of its members rounded up to a multiple of 16.
  - A mat4 matrix must have the same alignment as a vec4.
==========================================================================================================
*/

    glm::mat4 m_model;
    glm::mat4 m_view;
    glm::mat4 m_projection;
};

struct Model
{
    std::vector<Vertex> m_vertices;
    std::vector<uint32> m_indices;
};

struct Application
{

    uint32 m_currentFrame = 0;
    int32 m_joystick = -1;
   
    bool   m_framebufferResized = false;
    bool   m_vSync = false;

    glm::vec4 m_clearColor = glm::vec4(0.0f);
    real32 m_zoom = 2.0f;
    
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
    VkDescriptorSetLayout      m_descriptorSetLayout;
    VkPipelineLayout           m_pipelineLayout;
    VkPipeline                 m_graphicsPipline;
    VkCommandPool              m_commandPool;
    VkDescriptorPool           m_descriptorPool;

    VkImage        m_textureImage;
    VkDeviceMemory m_textureImageMemory;
    VkImageView    m_textureImageView;
    VkSampler      m_textureSampler;

    VkImage        m_depthImage;
    VkDeviceMemory m_depthImageMemory;
    VkImageView    m_depthImageView;
    
    VkBuffer                   m_vertexBuffer;
    VkDeviceMemory             m_vertexBufferMemory;
    VkBuffer                   m_indexBuffer;
    VkDeviceMemory             m_indexBufferMemory;

    Array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> m_descriptorSets;
    Array<VkBuffer,        MAX_FRAMES_IN_FLIGHT> m_uniformBuffers;
    Array<VkDeviceMemory,  MAX_FRAMES_IN_FLIGHT> m_uniformBuffersMemory;
    Array<void *,          MAX_FRAMES_IN_FLIGHT> m_uniformBuffersMapped;
    Array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> m_commandBuffers;

    std::vector<VkImage>       m_swapChainImages;
    std::vector<VkImageView>   m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    // NOTE: Synchronization Object
    Array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imageAvailableSemaphores;
    Array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_renderFinishedSemaphores;
    Array<VkFence, MAX_FRAMES_IN_FLIGHT>     m_inFlightFences;

    Model m_model;

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

struct ImageCreateResult
{
    VkImage        m_image;
    VkDeviceMemory m_imageMemory;
};

struct DepthResourcesCreateResult
{
    ImageCreateResult m_depthImageResult;
    VkImageView    m_depthImageView;
};

struct UniformBufferCreateResult
{
    Array<VkBuffer,       MAX_FRAMES_IN_FLIGHT> m_uniformBuffers;
    Array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> m_uniformBuffersMemory;
    Array<void *,         MAX_FRAMES_IN_FLIGHT> m_uniformBuffersMapped;
};

//====================================================
//      NOTE: Application Globals
//====================================================
#if 0
constexpr Vertex vertices[] =
{
    { {-0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
    { {-0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
    { {-0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
    { {-0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
};

constexpr uint16 vertexIndices[] =
{
    0, 1, 2, 2, 3, 0,
    0+4, 1+4, 2+4, 2+4, 3+4, 0+4,
};
#endif
//====================================================
//      NOTE: Application Functions
//====================================================
void RunApplication(Application & app);

internal glm::vec3 HexToRGB(uint32 val)
{
    uint32 r = ( val >> 16 ) & 0xFF;
    uint32 g = ( val >> 8  ) & 0xFF;
    uint32 b = ( val >> 0  ) & 0xFF;
    
    return glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);    
}

internal uint32 VertexHash(const Vertex & vertex)
{
    uint32 h1 = (uint32)std::hash<glm::vec3>()(vertex.m_pos);
    uint32 h2 = (uint32)std::hash<glm::vec3>()(vertex.m_color);
    uint32 h3 = (uint32)std::hash<glm::vec2>()(vertex.m_texCoord);

    return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1);
}

template<> struct std::hash<Vertex>
{
    size_t operator()(Vertex const& vertex) const
    {
        return VertexHash(vertex);
    }
};

#define APPLICATION_H
#endif
