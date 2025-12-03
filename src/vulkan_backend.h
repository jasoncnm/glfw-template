/* date = November 30th 2025 3:13 pm */
#ifndef VULKAN_BACKEND_H


#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "engine_lib.h"


#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>

//====================================================
//      NOTE: Vulkan Constexpr
//====================================================

constexpr char * validationLayers[] =
{
    "VK_LAYER_KHRONOS_validation"
};

constexpr char * deviceExtensions[] =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME    
};

constexpr int32 MAX_FRAMES_IN_FLIGHT = 2;

constexpr char * VS_PATH = "src/Shaders/bytecode/triangle_vert.spv";
constexpr char * FS_PATH = "src/Shaders/bytecode/triangle_frag.spv";

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
//      NOTE: Vulkan Structs
//====================================================

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

struct VulkanContext
{
    uint32 m_currentFrame = 0;
    
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
    VkPipeline                 m_graphicsPipeline;
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
    
     int64 m_shaderTimestamp;
    int64 m_textureTimestamp;
    int64 m_modelTimestamp;
    
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
    VkPipeline m_graphicsPipeline;
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

#define VULKAN_BACKEND_H
#endif //VULKAN_BACKEND_H
