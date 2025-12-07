#include "render_interface.h"
#include "vulkan_backend.h"

internal VkVertexInputBindingDescription GetVertexBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription = {};
    
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    return bindingDescription;
}

internal Array<VkVertexInputAttributeDescription, 3> GetVertexAttributeDescriptions()
{
    Array<VkVertexInputAttributeDescription, 3> attributeDescriptions(3);
    
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, m_pos);
    
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, m_color);
    
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, m_texCoord);
    
    return attributeDescriptions;
}

internal VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice,
                                      VkFormat * formats, uint32 formatCount,
                                      VkImageTiling tiling,
                                      VkFormatFeatureFlags features)
{
    for (uint32 i = 0; i < formatCount; i++)
    {
        VkFormat format = formats[i];
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
        
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
        
    }
    SM_ASSERT(false, "failed to find supported format");
    
    return (VkFormat)0;
}

internal VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice)
{
    VkFormat depthFormats[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    
    return FindSupportedFormat(physicalDevice, depthFormats, ArrayCount(depthFormats), tiling, features);
}

internal QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR  surface)
{
    QueueFamilyIndices indices;
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    for (int i = 0; i < queueFamilies.size(); i++)
    {
        const auto & queueFamily = queueFamilies[i];
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.m_graphicsFamily = i;            
        }
        
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport)
        {
            indices.m_presentFamily = i;
        }
        
        if (indices.IsComplete()) break;
    }
    
    return indices;
}

internal SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface)
{
    SwapChainSupportDetails result;
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &result.m_capabilities);
    
    uint32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    if (formatCount)
    {
        result.m_formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, result.m_formats.data());
    }
    
    uint32 presentModCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModCount, nullptr);
    if (presentModCount)
    {
        result.m_presentMods.resize(presentModCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModCount, result.m_presentMods.data());
    }
    
    return result;
}

internal VkResult CreateDebugUtilsMessengerEXT(
                                               VkInstance instance,
                                               const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator,
                                               VkDebugUtilsMessengerEXT* pDebugMessenger
                                               )
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

internal void DestroyDebugUtilsMessengerEXT(
                                            VkInstance instance,
                                            VkDebugUtilsMessengerEXT debugMessenger,
                                            const VkAllocationCallbacks* pAllocator
                                            )
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

internal VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback (
                                                       VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                       void* pUserData)
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        SM_ERROR("[VALIDATION]  %s", pCallbackData->pMessage);
         SM_ASSERT(false, "Bad Message!");
    }
    else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        // NOTE: Message is important enough to show
        SM_WARN("[VALIDATION]  %s", pCallbackData->pMessage);
    }
    else 
    {
        SM_TRACE("[VALIDATION]  %s", pCallbackData->pMessage);
    }
    
    return VK_FALSE;
}

internal VkDebugUtilsMessengerCreateInfoEXT
GetDebugMessengerCreateInfo()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    createInfo.pUserData = nullptr;
    
    return createInfo;
}

internal VkDebugUtilsMessengerEXT
SetupDebugMessenger(VkInstance instance)
{
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;    
    
    if (enableValidationLayers)
    {
        auto createInfo = GetDebugMessengerCreateInfo();
        
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        {
            SM_ASSERT(false, "[DEBUG_MSGER] failed to set up debug messenger!");
        }
    }
    
    return debugMessenger;
}

internal bool hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

internal bool CheckValidationLayerSupport()
{
    uint32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
    bool result = true;
    for (uint32 idx = 0; idx < ArrayCount(validationLayers); idx++)
    {
        bool layerFound = false;
        const char * layerName = validationLayers[idx];
        for (uint32 layerIdx = 0; layerIdx < availableLayers.size(); layerIdx++)
        {
            VkLayerProperties & layerProperties = availableLayers[layerIdx];
            
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        
        if (!layerFound)
        {
            result = false;
            break;
        }
    }
    
    return result;
}

internal std::vector<const char *> GetRequiredExtensions()
{
    uint32 glfwExtensionCount = 0;
    const char ** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    
    return extensions;
}

internal VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> & availableFormats)
{
    VkSurfaceFormatKHR result = availableFormats[0];
    
    //VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_PASS_THROUGH_EXT;
    VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    
    for (const auto & format : availableFormats)    
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace ==  colorSpace)
        {
            result = format;
            break;
        }
    }
    
    return result;
}

internal VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> & availablePresentModes)
{
    VkPresentModeKHR result = VK_PRESENT_MODE_FIFO_KHR;
    
    for (const auto & presentMode : availablePresentModes)
    {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            // NOTE: choose this mode if energy usage is not a concern (i.e Desktop PCs)
            result = presentMode;
            break;
        }
    }
    
    return result;
}

internal VkExtent2D ChooseSwapExtent(GLFWwindow * window, const VkSurfaceCapabilitiesKHR capabilities)
{
    VkExtent2D result;
    
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        result = capabilities.currentExtent;
    }
    else
    {
        int32 width, height;
        glfwGetFramebufferSize(window, &width, &height);
        result =
        {
            (uint32)width, (uint32)height
        };
        
        result.width = Clamp(result.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        result.height = Clamp(result.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }
    
    return result;
}

internal bool CheckDeviceExtensionSupport(const VkPhysicalDevice physicalDevice)
{
    
    uint32 extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());
    
    bool result = true;
    for (uint32 i = 0; i < ArrayCount(deviceExtensions); i++)
    {
        bool extensionFound = false;
        const char * extensionName = deviceExtensions[i];
        for (const auto & extension : availableExtensions)
        {
            if (strcmp(extension.extensionName, extensionName) == 0)
            {
                extensionFound = true;
                break;
            }
        }
        
        if (!extensionFound)
        {
            result = false;
            break;
        }
    }
    
    return result;
    
}

internal VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
    
    Array<VkDeviceQueueCreateInfo, 2> queueCreateInfos;
    Array<uint32, 2> uniqueQueueFamilies;
    
    uniqueQueueFamilies.Add(indices.m_graphicsFamily.value());
    if (indices.m_presentFamily.value() != indices.m_graphicsFamily.value())
    {
        uniqueQueueFamilies.Add(indices.m_presentFamily.value());
    }
    
    float queuePriority = 1.0f;
    for (uint32 i = 0; i < uniqueQueueFamilies.count; i++)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        
        queueCreateInfos.Add(queueCreateInfo);
    }
    
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.elements;
    createInfo.queueCreateInfoCount = (uint32)queueCreateInfos.count;
    createInfo.pEnabledFeatures = &deviceFeatures;
    
    createInfo.enabledExtensionCount = (uint32)ArrayCount(deviceExtensions);
    createInfo.ppEnabledExtensionNames = deviceExtensions;
    
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = (uint32)ArrayCount(validationLayers);
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }
    
    
    VkDevice device;
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        SM_ASSERT(false, "[DEVICE] failed to create logical device");
    }
    
    return device;    
}

internal VkQueue CreateGraphicsQueue(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
    
    VkQueue graphicsQueue;
    vkGetDeviceQueue(device, indices.m_graphicsFamily.value(), 0, &graphicsQueue);
    
    return graphicsQueue;
}

internal VkQueue CreatePresentQueue(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
    
    VkQueue presentQueue;
    vkGetDeviceQueue(device, indices.m_presentFamily.value(), 0, &presentQueue);
    
    return presentQueue;
    
}

internal VkSurfaceKHR CreateSurface(GLFWwindow * window, VkInstance instance)
{
    
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        SM_ASSERT(false, "[SURFACE] Failed to create window surface");
    }
    
    return surface;
}

internal CreateSwapChainResult CreateSwapChain(GLFWwindow * window, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice, surface);
    
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.m_formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.m_presentMods);
    VkExtent2D extent = ChooseSwapExtent(window, swapChainSupport.m_capabilities);
    
    uint32 imageCount = swapChainSupport.m_capabilities.minImageCount + 1;
    
    if (swapChainSupport.m_capabilities.maxImageCount > 0 && imageCount > swapChainSupport.m_capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.m_capabilities.maxImageCount;
    }
    
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    /*
      NOTE:
       - The imageUsage bit field specifies what kind of operations we'll use the images in the swap chain for.
       - VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT means render directly to them, which means that they're used as color attachment. 
     */
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
    uint32 queueFamilyIndices[] = { indices.m_graphicsFamily.value(), indices.m_presentFamily.value() };
    
    if (indices.m_graphicsFamily.value() != indices.m_presentFamily.value())
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    
    createInfo.preTransform = swapChainSupport.m_capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;    
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    VkSwapchainKHR swapChain = {};
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        SM_ASSERT(false, "Failed to create swap chain!");
    }
    
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    std::vector<VkImage> swapChainImages(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
    
    CreateSwapChainResult result;
    result.m_swapChain = swapChain;
    result.m_swapChainImages = swapChainImages;
    result.m_swapChainImageFormat = surfaceFormat.format;
    result.m_swapChainExtent = extent;
    
    return result;
}

internal VkImageView CreateImageView(VkDevice device, 
                                     VkImage image,
                                     VkFormat format,
                                     VkImageAspectFlags aspectFlags,
                                     uint32 mipLevels)
{
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = mipLevels;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    
    VkImageView result;
    if (vkCreateImageView(device, &createInfo, nullptr, &result) != VK_SUCCESS)
    {
        SM_ASSERT(false, "Failed to create Image view");
    }
    
    return result;    
}

internal std::vector<VkImageView> CreateImageViews(std::vector<VkImage> & swapChainImages, VkDevice device, VkFormat swapChainImageFormat)
{
    std::vector<VkImageView> result(swapChainImages.size());
    
    for (uint32 i = 0; i < swapChainImages.size(); i++)
    {
        result[i] = CreateImageView(device, swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
    
    return result;
}

internal VkImageView CreateTextureImageView(VkDevice device, VkImage image, uint32 mipLevels)
{
    
    VkImageView imageView = CreateImageView(device, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
    
    return imageView;
}

// IMPORTANT: Make sure the byte code is null terminated
internal VkShaderModule CreateShaderModule(VkDevice device, std::vector<char> & code)
{
    SM_ASSERT(code.size() > 0, "file readed are empty");
    
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() - 1;
    createInfo.pCode = (uint32_t *)code.data();
    
    VkShaderModule result;
    
    if (vkCreateShaderModule(device, &createInfo, nullptr, &result) != VK_SUCCESS)
    {
        SM_ASSERT(false, "Failed to create shader module");
    }
    
    return result;
}


/*
  ============================IMPORTANT=============================
                        Graphics Pipeline
  ==================================================================
  https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Introduction   
  ==================================================================
 -- (input) Vertex/Index buffer 
-> Input Assembler (fixed funciton)  -> Vertex Shader 
-> Tessellation        -> Geomeetry shader -> Rasterization(fixed funciton)
 -> Fragment Shader     -> Color blending(fixed funciton)
 -> (output) Framebuffer
  ==================================================================
*/
internal CreateGraphicsPipelineResult
CreateGraphicsPipeline(VkDevice device, VkExtent2D swapChainExtent, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout)
{
    // NOTE: This is null terminated
    std::vector<char> vertShaderCode = read_file(VS_PATH);
    std::vector<char> fragShaderCode = read_file(FS_PATH);
    
    VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);    
    
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    
    
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
    
    // NOTE: Vertex Input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    
    VkVertexInputBindingDescription bindingDescription = GetVertexBindingDescription();
    Array<VkVertexInputAttributeDescription, 3> attributeDescriptions = GetVertexAttributeDescriptions();
    
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.count;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.elements;
    
    // NOTE: Input Assembly 
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    
    // NOTE: Viewports and scissors
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;
    
    // NOTE: Specify Viewports and scissors rect as Dynamic State 
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = (uint32)ArrayCount(dynamicStates);
    dynamicState.pDynamicStates = dynamicStates;
    
    VkPipelineViewportStateCreateInfo viewPortState = {};
    viewPortState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewPortState.viewportCount = 1;
    viewPortState.scissorCount = 1;
    
    // NOTE: Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional    
    
    // NOTE: MultiSampling
    VkPipelineMultisampleStateCreateInfo multiSampling = {};
    multiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multiSampling.sampleShadingEnable = VK_FALSE;
    multiSampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multiSampling.minSampleShading = 1.0f; // Optional
    multiSampling.pSampleMask = nullptr; // Optional
    multiSampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multiSampling.alphaToOneEnable = VK_FALSE; // Optional
    
    // NOTE: Color Blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    
    //NOTE alpha blending    
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;  // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional
    
    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional
    
    
    // NOTE: Pipeline Layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;            
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    
    VkPushConstantRange vertPushConst = {};
    vertPushConst.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    vertPushConst.offset = 0;
    vertPushConst.size = sizeof(VertPushConstants);
    
    VkPushConstantRange fragPushConst = {};
    fragPushConst.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragPushConst.offset = sizeof(VertPushConstants);
    fragPushConst.size = sizeof(FragPushConstants);
    
    VkPushConstantRange pushConstants[] = { vertPushConst, fragPushConst };
     pipelineLayoutInfo.pushConstantRangeCount = ArrayCount(pushConstants);    
    pipelineLayoutInfo.pPushConstantRanges = pushConstants;
     
    VkPipelineLayout layout;
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to create pipeline layout!");
    }
    
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewPortState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multiSampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = layout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional
    
    VkPipeline graphicsPipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to create graphics pipeline!");
    }
    
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    
    CreateGraphicsPipelineResult result = { layout, graphicsPipeline };
    
    return result;
}

internal VkRenderPass CreateRenderPass(VkDevice device, VkPhysicalDevice physicalDevice, VkFormat imageFormat)
{
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat(physicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = imageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
    VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = ArrayCount(attachments);
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    VkRenderPass renderPass;
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to create render pass");
    }
    
    return renderPass;
    
}

internal std::vector<VkFramebuffer>
CreateFramebuffers(VkDevice device,
                   std::vector<VkImageView> & imageViews,
                   VkImageView depthImageView,
                   VkRenderPass renderPass,
                   VkExtent2D extent)
{
    std::vector<VkFramebuffer> framebuffers(imageViews.size());
    
    for (int i = 0; i < imageViews.size(); i++)
    {
        VkImageView attachments[] = { imageViews[i], depthImageView };
        
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = ArrayCount(attachments);
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;
        
        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
        {
            SM_ASSERT(false, "failed to create frame buffer!");
        }
        
    }
    return framebuffers;
}

internal VkCommandPool CreateCommandPool(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice, surface);
    
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.m_graphicsFamily.value();
    
    VkCommandPool pool;
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to create command pool");
    }
    
    return pool;
}

internal Array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT>
CreateCommandBuffers(VkDevice device, VkCommandPool commandPool)
{
    Array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> commandBuffers;
    commandBuffers.Resize(MAX_FRAMES_IN_FLIGHT);
    
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = commandBuffers.count;
    
    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.elements) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to allocate command buffers!");
    }
    
    return commandBuffers;    
}

internal VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
{
    
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    
    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to allocate command buffers!");
    }
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to begine recording command buffer!");
    }
    
    return commandBuffer;    
}

internal void EndSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer, VkQueue graphicsQueue, VkCommandPool commandPool)
{
    
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to record command buffer!");
    }
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to submit draw command buffer!");
    }
    vkQueueWaitIdle(graphicsQueue);
    
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

internal SyncObjects
CreateSyncObjects(VkDevice device)
{
    Array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
    Array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
    Array<VkFence, MAX_FRAMES_IN_FLIGHT>     inFlightFences;
    
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    imageAvailableSemaphores.Resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.Resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.Resize(MAX_FRAMES_IN_FLIGHT);
    
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS     ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            SM_ASSERT(false, "failed to create syncronization objects");
        }
    }
    
    SyncObjects result =
    {
        imageAvailableSemaphores,
        renderFinishedSemaphores,
        inFlightFences
    };
    
    return result;
}

internal void CleanupSwapChain(VulkanContext & context)
{
    vkDestroyImageView(context.m_device, context.m_depthImageView, nullptr);
    vkDestroyImage(context.m_device, context.m_depthImage, nullptr);
    vkFreeMemory(context.m_device, context.m_depthImageMemory, nullptr);
    
    for (uint32 i = 0; i < context.m_swapChainFramebuffers.size(); i++)
    {
        vkDestroyFramebuffer(context.m_device, context.m_swapChainFramebuffers[i], nullptr);
    }
    
    for (uint32 i = 0; i < context.m_swapChainImageViews.size(); i++)
    {
        vkDestroyImageView(context.m_device, context.m_swapChainImageViews[i], nullptr);
    }
    
    vkDestroySwapchainKHR(context.m_device, context.m_swapChain, nullptr);
    
}

internal uint32 FindMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    for (uint32 i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    
    SM_ASSERT(false, "failed to find suitable memory type!");
    
    return 0;
}

internal BufferCreateResult
CreateBuffer(VkDevice device,
             VkPhysicalDevice physicalDevice,
             VkDeviceSize size,
             VkBufferUsageFlags usage,
             VkMemoryPropertyFlags properties)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkBuffer buffer = {};
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to create vertex buffer");
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryType(physicalDevice,
                                                  memRequirements.memoryTypeBits, properties);
    
    VkDeviceMemory bufferMemory;
    if (vkAllocateMemory(device, &allocateInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to allocate vertex buffer memory!");
    }
    
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
    
    BufferCreateResult result = { buffer, bufferMemory };
    
    return result;
}

internal void CopyBuffer(VkDevice device,
                         VkCommandPool commandPool,
                         VkQueue graphicsQueue,
                         VkBuffer srcBuffer,
                         VkBuffer dstBuffer,
                         VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);
    
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    
    // IMPORTANT
    // srcBuffer must have been created with VK_BUFFER_USAGE_TRANSFER_SRC_BIT usage flag
    // dstBuffer must have been created with VK_BUFFER_USAGE_TRANSFER_DST_BIT usage flag
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    
    EndSingleTimeCommands(device, commandBuffer, graphicsQueue, commandPool);
}

internal void CopyBufferToImage(VkDevice device,
                                VkCommandPool commandPool,
                                VkQueue graphicsQueue,
                                VkBuffer buffer,
                                VkImage image,
                                uint32 width,
                                uint32 height)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);
    
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };
    
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    
    EndSingleTimeCommands(device, commandBuffer, graphicsQueue, commandPool);
}

internal ImageCreateResult CreateImage(VkDevice device,
                                       VkPhysicalDevice physicalDevice,
                                       uint32 width,
                                       uint32 height,
                                       uint32 mipLevels,
                                       VkFormat format,
                                       VkImageTiling tiling,
                                       VkImageUsageFlags usage,
                                       VkMemoryPropertyFlags properties)
{
    
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage; 
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optional
    
    VkImage image = {};
    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to create image!");
    }
    
    VkMemoryRequirements memRequirements = {};
    vkGetImageMemoryRequirements(device, image, &memRequirements);
    
    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryType(physicalDevice,
                                                  memRequirements.memoryTypeBits, properties);
    
    VkDeviceMemory imageMemory;
    if (vkAllocateMemory(device, &allocateInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to allocate vertex buffer memory!");
    }
    
    vkBindImageMemory(device, image, imageMemory, 0);
    
    ImageCreateResult result = { image, imageMemory };
    
    return result;
}

internal void TransitionImageLayout(VkDevice device,
                                    VkCommandPool commandPool,
                                    VkQueue graphicsQueue,
                                    VkImage  image,
                                    VkFormat format,
                                    uint32 mipLevels,
                                    VkImageLayout oldLayout,
                                    VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);
    
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    // NOTE: If you are using the barrier to transfer queue family ownership,
    //       then these two fields should be the indices of the queue families.
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    VkPipelineStageFlags srcStage = {};
    VkPipelineStageFlags dstStage = {};
    
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        SM_ASSERT(false, "unsupported layout transition!");
    }
    
    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    
    EndSingleTimeCommands(device, commandBuffer, graphicsQueue, commandPool);
}

internal void GenerateMipMaps(VkDevice device,
                              VkPhysicalDevice physicalDevice, 
                              VkCommandPool commandPool,
                              VkQueue graphicsQueue, 
                              VkImage image,
                              VkFormat imageFormat, 
                              int32 texWidth,
                              int32 texHeight,
                              uint32 mipLevels)
{
    // NOTE: Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        SM_ASSERT(false, "texture image format does not support linear bliting!");
        
        /*
 NOTE: 
There are two alternatives in this case. You could implement a function that searches common texture image formats for one that does support linear blitting, or you could implement the mipmap generation in software with a library like stb_image_resize. Each mip level can then be loaded into the image in the same way that you loaded the original image.
It should be noted that it is uncommon in practice to generate the mipmap levels at runtime anyway. Usually they are pregenerated and stored in the texture file alongside the base level to improve loading speed.
    */
    }
    
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;
    
    int32 mipWidth = texWidth;
    int32 mipHeight = texHeight;
    for (uint32 i = 1; i < mipLevels; i++)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);
        
        VkImageBlit blit = {};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        
        vkCmdBlitImage(commandBuffer,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);
        
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
                             0, 0, nullptr, 0, nullptr,1, &barrier);
        
        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }
    
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
                         0, 0, nullptr, 0, nullptr,1, &barrier);
    
    EndSingleTimeCommands(device, commandBuffer, graphicsQueue, commandPool);
}

internal ImageCreateResult
CreateTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const char * texturePath)
{
    // Standard parameters:
    //    int *x                 -- outputs image width in pixels
    //    int *y                 -- outputs image height in pixels
    //    int *channels_in_file  -- outputs # of image components in image file
    //    int desired_channels   -- if non-zero, # of image components requested in result
    int x,y, channels_in_file;
    unsigned char * imageData = stbi_load(texturePath, &x, &y, &channels_in_file, STBI_rgb_alpha);
    SM_ASSERT(imageData, "failed to load texture image!");
    uint32 mipLevels = (uint32)(std::floor(std::log2(max(x, y)))) + 1;
    
    VkDeviceSize imageSize = x * y * 4;
    
    
    BufferCreateResult stagingBufferResult = CreateBuffer(device,
                                                          physicalDevice,
                                                          imageSize,
                                                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    void * data;
    vkMapMemory(device, stagingBufferResult.m_bufferMemory, 0, imageSize, 0, &data);
    memcpy(data, imageData, (uint32)imageSize);
    vkUnmapMemory(device, stagingBufferResult.m_bufferMemory);
    stbi_image_free(imageData);
    
    ImageCreateResult textureImageResult = CreateImage(device,
                                                       physicalDevice,
                                                       (uint32)x,
                                                       (uint32)y,
                                                       mipLevels,
                                                       VK_FORMAT_R8G8B8A8_SRGB,
                                                       VK_IMAGE_TILING_OPTIMAL,
                                                       VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    TransitionImageLayout(device,
                          commandPool, 
                          graphicsQueue,
                          textureImageResult.m_image,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          mipLevels, 
                          VK_IMAGE_LAYOUT_UNDEFINED, 
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
    CopyBufferToImage(device, commandPool, graphicsQueue, stagingBufferResult.m_buffer, textureImageResult.m_image, (uint32)x, (uint32)y);
    
    // NOTE: transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
    GenerateMipMaps(device, physicalDevice, commandPool, graphicsQueue, textureImageResult.m_image, VK_FORMAT_R8G8B8A8_SRGB,  x, y, mipLevels);
    
    vkDestroyBuffer(device, stagingBufferResult.m_buffer, nullptr);
    vkFreeMemory(device, stagingBufferResult.m_bufferMemory, nullptr);
    
    textureImageResult.m_mipLevels = mipLevels;
    
    return textureImageResult;
    
}

internal VkSampler CreateTextureSampler(VkDevice device, VkPhysicalDevice physicalDevice)
{
    
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    
    // NOTE: Bilinear filtering
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    
    // NOTE: Repeat the texture when going beyond the image dimensions.
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    
    // NOTE: Enable anisotropic filtering
    samplerInfo.anisotropyEnable = VK_TRUE;
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE; // NOTE: texels are addressed using the [0, 1) range on all axes
    
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod     = 0.0f;
        samplerInfo.maxLod     = VK_LOD_CLAMP_NONE;
    
    VkSampler sampler = {};
    if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to create texture sampler!");
    }
    
    return sampler;
}

internal DepthResourcesCreateResult CreateDepthResources(VkDevice device, VkPhysicalDevice physicalDevice, VkExtent2D extent)
{
    VkFormat depthFormat = FindDepthFormat(physicalDevice);
    
    ImageCreateResult depthImageResult = CreateImage(device, physicalDevice, extent.width, extent.height, 1, depthFormat,
                                                     VK_IMAGE_TILING_OPTIMAL,
                                                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    VkImageView depthImageView = CreateImageView(device, depthImageResult.m_image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    
    DepthResourcesCreateResult result = {};
    result.m_depthImageResult = depthImageResult;
    result.m_depthImageView = depthImageView;
    
    return result;
}

internal BufferCreateResult
CreateAndBindVertexBuffer(VkDevice device,
                          VkCommandPool commandPool,
                          VkQueue graphicsQueue,
                          VkPhysicalDevice physicalDevice,
                          std::vector<Vertex> & vertices)
{
    
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    BufferCreateResult staginBufferResult = CreateBuffer(device,
                                                         physicalDevice,
                                                         bufferSize,
                                                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    void * data;
    // NOTE: memory must have been created with a memory type that reports VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    vkMapMemory(device, staginBufferResult.m_bufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (uint32)bufferSize);
    vkUnmapMemory(device, staginBufferResult.m_bufferMemory);
    
    BufferCreateResult vertexBufferResult = CreateBuffer(device,
                                                         physicalDevice,
                                                         bufferSize,
                                                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    CopyBuffer(device, commandPool, graphicsQueue, staginBufferResult.m_buffer, vertexBufferResult.m_buffer, bufferSize);
    
    vkDestroyBuffer(device, staginBufferResult.m_buffer, nullptr);
    vkFreeMemory(device, staginBufferResult.m_bufferMemory, nullptr);
    
    return vertexBufferResult;
}

internal BufferCreateResult
CreateAndBindIndexBuffer(VkDevice device,
                         VkCommandPool commandPool,
                         VkQueue graphicsQueue,
                         VkPhysicalDevice physicalDevice,
                         std::vector<uint32> & indices)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
    BufferCreateResult staginBufferResult = CreateBuffer(device,
                                                         physicalDevice,
                                                         bufferSize,
                                                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    void * data;
    vkMapMemory(device, staginBufferResult.m_bufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (uint32)bufferSize);
    vkUnmapMemory(device, staginBufferResult.m_bufferMemory);
    
    BufferCreateResult indexBufferResult = CreateBuffer(device,
                                                        physicalDevice,
                                                        bufferSize,
                                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    CopyBuffer(device, commandPool, graphicsQueue, staginBufferResult.m_buffer, indexBufferResult.m_buffer, bufferSize);
    
    vkDestroyBuffer(device, staginBufferResult.m_buffer, nullptr);
    vkFreeMemory(device, staginBufferResult.m_bufferMemory, nullptr);
    
    return indexBufferResult;
}

internal UniformBufferCreateResult
CreateUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice)
{
    UniformBufferCreateResult result = {};
    
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    
    for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        BufferCreateResult bufferResult = CreateBuffer(device,
                                                       physicalDevice,
                                                       bufferSize,
                                                       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        result.m_uniformBuffers.Add(bufferResult.m_buffer);
        result.m_uniformBuffersMemory.Add(bufferResult.m_bufferMemory);
        
        void * data;
        vkMapMemory(device, bufferResult.m_bufferMemory, 0, bufferSize, 0, &data);
        result.m_uniformBuffersMapped.Add(data);
    }
    
    return result;
}

internal VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  // only vertex shader be reference to the ubo
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional relevant for image sampling related descriptors
    
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    VkDescriptorSetLayoutBinding bindings[] = { uboLayoutBinding, samplerLayoutBinding };
    
    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = ArrayCount(bindings);
    createInfo.pBindings = bindings;
    
    VkDescriptorSetLayout result;
    if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &result) != VK_SUCCESS)
    {
        SM_ASSERT(false, "faile to create descriptor set layout!");
    }
    
    return result;
}

internal VkDescriptorPool CreateDescriptorPool(VkDevice device, uint32 textureCount)
{
    VkDescriptorPoolSize poolSizes[] = 
    {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (uint32)MAX_FRAMES_IN_FLIGHT },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32)MAX_FRAMES_IN_FLIGHT * textureCount },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE }
    };
    
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 0;
    for ( int i = 0; i < ArrayCount(poolSizes); i++)
    {
        VkDescriptorPoolSize & poolSize = poolSizes[i];
        poolInfo.maxSets += poolSize.descriptorCount;
    }
    poolInfo.poolSizeCount = ArrayCount(poolSizes);
    poolInfo.pPoolSizes = poolSizes;
    
    
    VkDescriptorPool pool;
    
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to create descriptor pool!");
    }
    
    return pool;
    
}

internal Array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>
CreateDescriptorSets(VkDevice device,
                     Array<VkBuffer, MAX_FRAMES_IN_FLIGHT> uniformBuffers,
                     VkDescriptorPool descriptorPool,
                     VkDescriptorSetLayout descriptorSetLayout,
                     VkImageView textureImageView,
                     VkSampler textureSampler)
{
    Array<VkDescriptorSetLayout, MAX_FRAMES_IN_FLIGHT> layouts(MAX_FRAMES_IN_FLIGHT);
    layouts.Fill(descriptorSetLayout);
    
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = (uint32)MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts.elements;
    
    Array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSets(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.elements) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to allocate descriptor sets!");
    }
    
    for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);
        
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView   = textureImageView;
        imageInfo.sampler     = textureSampler;
        
        VkWriteDescriptorSet descriptorWrites[2] = {};
        
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;
        descriptorWrites[0].pImageInfo = nullptr; // Optional
        descriptorWrites[0].pTexelBufferView = nullptr; // Optional
        
        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = nullptr; // Optional
        descriptorWrites[1].pImageInfo = &imageInfo;
        descriptorWrites[1].pTexelBufferView = nullptr; // Optional
        
        vkUpdateDescriptorSets(device, ArrayCount(descriptorWrites), descriptorWrites, 0, nullptr);
    }
    
    return descriptorSets;
}

internal IsDeviceSuitableResult IsDeviceSuitable(const VkPhysicalDevice device, const VkSurfaceKHR surface)
{
    
    IsDeviceSuitableResult result = {};
    
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    
    QueueFamilyIndices indices = FindQueueFamilies(device, surface);
    
    bool extensionsSupported = CheckDeviceExtensionSupport(device);
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.m_presentMods.empty() && !swapChainSupport.m_formats.empty();
    }
    
    result.m_isSuitable = indices.IsComplete() && extensionsSupported && swapChainAdequate && deviceFeatures.samplerAnisotropy;
    
    /*
      ========================================IMPORTANT============================================
      Current implementation prefer picking discrete GPU 
      =============================================================================================
      Give each device a score and pick the highest one.
      Favor a dedicated graphics card by giving it a higher score,
      but fall back to an integrated GPU if that's the only available one. 
      =============================================================================================
    */
    
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) result.m_score++;
    if (deviceFeatures.geometryShader) result.m_score++;
    
    return result;
}

internal VkPhysicalDevice PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
    
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    
    uint32 deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    
    if (!deviceCount)
    {
        SM_ASSERT(false, "[PHYSICAL_DEVICE] failed to find GPUs with Vulkan support!");        
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    
    int32 maxScore = -1;
    for (const auto & device : devices)
    {
        IsDeviceSuitableResult result = IsDeviceSuitable(device, surface);
        if (result.m_isSuitable && result.m_score > maxScore)
        {
            physicalDevice = device;
            maxScore = result.m_score;
        }
    }
    
    if (physicalDevice == VK_NULL_HANDLE)
    {
        SM_ASSERT(false, "[PHYSICAL_DEVICE] failed to find a suitable GPU!");        
    }
    
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        SM_TRACE("[PHYSICAL_DEVICE] [SELETED_DEVICE] %s", deviceProperties.deviceName); 
    }
    
    return physicalDevice;
}


internal VkInstance CreateVkInstance()
{
    if (enableValidationLayers && !CheckValidationLayerSupport())
    {
        SM_ASSERT(false, "validation layers requested, but not available!");
    }
    
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "application template";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    auto extensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = (uint32)extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    
    // NOTE: Declear outside if statment to ensure it is not destroyed before vkCreateInstance
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    
    // NOTE: Include the validation layer
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = (uint32)(ArrayCount(validationLayers));
        createInfo.ppEnabledLayerNames = validationLayers;
        
        debugCreateInfo = GetDebugMessengerCreateInfo();
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
    
    VkInstance instance;
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    
    if (result != VK_SUCCESS)
    {
        SM_ASSERT(false, "[INSTANCE] failed to create instance!");
    }
    
    return instance;
}

internal void RecreateSwapChain(GLFWwindow* window, VulkanContext & context)
{
    int32 width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    for ( ; width == 0 || height == 0; )
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(context.m_device);
    
    CleanupSwapChain(context);
    
    // NOTE: swapchain, images, format, extent creation
    {
        CreateSwapChainResult createResult = CreateSwapChain(window, context.m_device, context.m_physicalDevice, context.m_surface);
        context.m_swapChain = createResult.m_swapChain;
        context.m_swapChainImages = createResult.m_swapChainImages;
        context.m_swapChainImageFormat = createResult.m_swapChainImageFormat;
        context.m_swapChainExtent = createResult.m_swapChainExtent;
    }
    context.m_swapChainImageViews = CreateImageViews(context.m_swapChainImages, context.m_device, context.m_swapChainImageFormat);
    
    
    {
        DepthResourcesCreateResult result = CreateDepthResources(context.m_device, context.m_physicalDevice, context.m_swapChainExtent);
        
        context.m_depthImage       = result.m_depthImageResult.m_image;
        context.m_depthImageMemory = result.m_depthImageResult.m_imageMemory;
        context.m_depthImageView   = result.m_depthImageView;
    }
    
    
    context.m_swapChainFramebuffers = CreateFramebuffers(context.m_device, context.m_swapChainImageViews, context.m_depthImageView, context.m_renderPass, context.m_swapChainExtent);
}

internal void UpdateUniformBuffer(VulkanContext & context, RenderData * renderData)
{
    UniformBufferObject ubo = {};
    // Control view and projection matrix based on camera position forwardDirection, 
    // fov, zoom, and near/far clip
    
    Camera & cam = renderData->m_camera;
    ubo.m_view = glm::lookAt(cam.m_pos,
                             cam.m_pos + cam.m_forwardDirection, 
                             glm::vec3(0.0f, 0.0f, 1.0f));
    
    real32 aspect = (real32)context.m_swapChainExtent.width / (real32)context.m_swapChainExtent.height;
    ubo.m_projection = glm::perspective(glm::radians(renderData->m_camera.m_fovy),
                                        aspect, 
                                        renderData->m_camera.m_nearClip,
                                        renderData->m_camera.m_farClip);
    ubo.m_projection[1][1] *= -1;
    
    memcpy(context.m_uniformBuffersMapped[context.m_currentFrame], &ubo, sizeof(ubo));
}



internal
void RecordCommandBuffer(VulkanContext & context, RenderData * renderData, uint32 imageIndex)
{
    VkCommandBuffer & commandBuffer = context.m_commandBuffers[context.m_currentFrame];
    
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
    renderPassInfo.renderPass = context.m_renderPass;
    renderPassInfo.framebuffer = context.m_swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = context.m_swapChainExtent;
    
    VkClearValue clearColor =  { { renderData->m_clearColor.r, renderData->m_clearColor.g, renderData->m_clearColor.b, renderData->m_clearColor.a } };
    VkClearValue clearDepthStencil = { 1.0f, 0 };
    
    VkClearValue clearValues[] = { clearColor, clearDepthStencil };
    renderPassInfo.clearValueCount = ArrayCount(clearValues);
    renderPassInfo.pClearValues = clearValues;
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context.m_graphicsPipeline);
    
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (real32)context.m_swapChainExtent.width;
    viewport.height = (real32)context.m_swapChainExtent.height;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    
    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = context.m_swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    
    for (uint32 i = 0; i < renderData->m_transforms.count; i++)
    {
        Transform & transform = renderData->m_transforms[i];
        ModelContext & modelContext = context.m_modelContexts[transform.m_modelID];
        TextureContext & textureContext = context.m_textureContexts[transform.m_textureID];
        
    VkBuffer vertexBuffers[] = { modelContext.m_vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    
    vkCmdBindIndexBuffer(commandBuffer, modelContext.m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            context.m_pipelineLayout,
                            0,
                            1,
                            &textureContext.m_descriptorSets[context.m_currentFrame],
                            0,
                            nullptr);
    
    FragPushConstants fragConsts = {};
    fragConsts.m_viewDistence = renderData->m_fog.m_viewDistence;
    fragConsts.m_steepness = renderData->m_fog.m_steepness;
    fragConsts.m_fogColor = renderData->m_fog.m_fogColor;
    
    vkCmdPushConstants(commandBuffer,
                       context.m_pipelineLayout, 
                       VK_SHADER_STAGE_FRAGMENT_BIT, 
                       sizeof(VertPushConstants), sizeof(fragConsts), 
                       &fragConsts);
    
    for (glm::vec3 meshPosition : transform.m_meshPositions)
    {
        VertPushConstants meshConstants = {};
        meshConstants.m_model = glm::translate(glm::mat4(1.0), meshPosition);
        vkCmdPushConstants(commandBuffer,
                           context.m_pipelineLayout, 
                           VK_SHADER_STAGE_VERTEX_BIT, 
                           0, sizeof(meshConstants), 
                           &meshConstants);
        vkCmdDrawIndexed(commandBuffer, (uint32)transform.m_model.m_indices.size(), 1, 0, 0, 0);
    }
    }
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    
    vkCmdEndRenderPass(commandBuffer);
    
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to record command buffer!");
    }
}


Model LoadModel();
internal void InitVulkan(Application & app)
{
    VulkanContext & context = app.m_renderContext;
    context.m_instance       = CreateVkInstance();
    context.m_debugMessenger = SetupDebugMessenger(context.m_instance);
    context.m_surface        = CreateSurface(app.m_window, context.m_instance);
    context.m_physicalDevice = PickPhysicalDevice(context.m_instance, context.m_surface);
    context.m_device         = CreateLogicalDevice(context.m_physicalDevice, context.m_surface);
    context.m_graphicsQueue  = CreateGraphicsQueue(context.m_device, context.m_physicalDevice, context.m_surface);
    context.m_presentQueue   = CreatePresentQueue(context.m_device, context.m_physicalDevice, context.m_surface);
    
    // NOTE: swapchain, images, format, extent creation
    {
        CreateSwapChainResult createResult = CreateSwapChain(app.m_window, context.m_device, context.m_physicalDevice, context.m_surface);
        
        context.m_swapChain            = createResult.m_swapChain;
        context.m_swapChainImages      = createResult.m_swapChainImages;
        context.m_swapChainImageFormat = createResult.m_swapChainImageFormat;
        context.m_swapChainExtent      = createResult.m_swapChainExtent;
    }
    
    context.m_swapChainImageViews = CreateImageViews(context.m_swapChainImages, context.m_device, context.m_swapChainImageFormat);
    context.m_renderPass          = CreateRenderPass(context.m_device, context.m_physicalDevice, context.m_swapChainImageFormat);
    context.m_descriptorSetLayout = CreateDescriptorSetLayout(context.m_device);
    
    {
        CreateGraphicsPipelineResult result =
            CreateGraphicsPipeline(context.m_device, context.m_swapChainExtent, context.m_renderPass, context.m_descriptorSetLayout);
        
        context.m_pipelineLayout  = result.m_pipelineLayout;
        context.m_graphicsPipeline = result.m_graphicsPipeline;
    }
    
    context.m_commandPool = CreateCommandPool(context.m_device, context.m_physicalDevice, context.m_surface);
    
    {
        for (uint32 i = 0; i < app.m_renderData.m_transforms.count; i++)
        {
            Transform & tr = app.m_renderData.m_transforms[i];
            if (context.m_textureContexts.find(tr.m_textureID) != context.m_textureContexts.end())
            {
                continue;
            }
                                        ImageCreateResult result = CreateTextureImage(context.m_device, 
                                                      context.m_physicalDevice, 
                                                      context.m_commandPool, 
                                                      context.m_graphicsQueue, 
                                                          tr.m_textureID);
            
            TextureContext texture = {};
            texture.m_textureImage       = result.m_image;
            texture.m_textureImageMemory = result.m_imageMemory;
            texture.m_mipLevels          = result.m_mipLevels;
            texture.m_textureImageView = CreateTextureImageView(context.m_device, texture.m_textureImage, texture.m_mipLevels);
            context.m_textureContexts[tr.m_textureID] = texture;
            }
        
        context.m_textureSampler   = CreateTextureSampler(context.m_device, context.m_physicalDevice);
        }
    
    {
        DepthResourcesCreateResult result = CreateDepthResources(context.m_device, context.m_physicalDevice, context.m_swapChainExtent);
        
        context.m_depthImage       = result.m_depthImageResult.m_image;
        context.m_depthImageMemory = result.m_depthImageResult.m_imageMemory;
        context.m_depthImageView   = result.m_depthImageView;
    }
    
    context.m_swapChainFramebuffers = CreateFramebuffers(context.m_device, context.m_swapChainImageViews, context.m_depthImageView, context.m_renderPass, context.m_swapChainExtent);
    
    for (uint32 i = 0; i < app.m_renderData.m_transforms.count; i++)
    {
        Transform & tr = app.m_renderData.m_transforms[i];
        ModelContext & modelContext = context.m_modelContexts[tr.m_modelID];
        {
        BufferCreateResult result = CreateAndBindVertexBuffer(context.m_device, context.m_commandPool, context.m_graphicsQueue, context.m_physicalDevice, tr.m_model.m_vertices);
            modelContext.m_vertexBuffer        = result.m_buffer;
            modelContext.m_vertexBufferMemory  = result.m_bufferMemory;
    }
    
    {
        BufferCreateResult result = CreateAndBindIndexBuffer(context.m_device, context.m_commandPool, context.m_graphicsQueue, context.m_physicalDevice, tr.m_model.m_indices);
            modelContext.m_indexBuffer         = result.m_buffer;
            modelContext.m_indexBufferMemory   = result.m_bufferMemory;
    }
    }
    
    {
        UniformBufferCreateResult result = CreateUniformBuffers(context.m_device, context.m_physicalDevice);
        context.m_uniformBuffers       = result.m_uniformBuffers;
        context.m_uniformBuffersMemory = result.m_uniformBuffersMemory;
        context.m_uniformBuffersMapped = result.m_uniformBuffersMapped;
    }
    
    context.m_descriptorPool = CreateDescriptorPool(context.m_device, (uint32)context.m_textureContexts.size());
    
    for (uint32 i = 0; i < app.m_renderData.m_transforms.count; i++)
    {
        Transform & tr = app.m_renderData.m_transforms[i];
        TextureContext & textureContext = context.m_textureContexts[tr.m_textureID];
        textureContext.m_descriptorSets = CreateDescriptorSets(context.m_device,
                                                context.m_uniformBuffers,
                                                context.m_descriptorPool,
                                                context.m_descriptorSetLayout,
                                                textureContext.m_textureImageView,
                                                context.m_textureSampler);
    }
    
    context.m_commandBuffers = CreateCommandBuffers(context.m_device, context.m_commandPool);
    
    {
        SyncObjects syncObjs           = CreateSyncObjects(context.m_device);
        context.m_imageAvailableSemaphores = syncObjs.m_imageAvailableSemaphores;
        context.m_renderFinishedSemaphores = syncObjs.m_renderFinishedSemaphores;
        context.m_inFlightFences           = syncObjs.m_inFlightFences;
    }
    
    int64 timestampVS = GetTimestamp(VS_PATH);
    int64 timestampFS = GetTimestamp(FS_PATH);
    context.m_shaderTimestamp = max(timestampVS, timestampFS);
}

internal void RecreateGrahpicsPipeline(VulkanContext & context)
{
    vkDeviceWaitIdle(context.m_device);
    
    vkDestroyPipeline(context.m_device, context.m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(context.m_device, context.m_pipelineLayout, nullptr);
    
    CreateGraphicsPipelineResult result =
        CreateGraphicsPipeline(context.m_device, context.m_swapChainExtent, context.m_renderPass, context.m_descriptorSetLayout);
    
    context.m_pipelineLayout  = result.m_pipelineLayout;
    context.m_graphicsPipeline = result.m_graphicsPipeline;
    
}

internal void DrawFrame(Application & app, RenderData * renderData)
{
    VulkanContext & context = app.m_renderContext;
    vkWaitForFences(context.m_device, 1, &context.m_inFlightFences[context.m_currentFrame], VK_TRUE, UINT64_MAX);
    
    {
    int64 timestampVS = GetTimestamp(VS_PATH);
    int64 timestampFS = GetTimestamp(FS_PATH);
    int64 currentTimeStamp = max(timestampVS, timestampFS);
        if (KeyIsDown(app.m_input, GLFW_KEY_R) && currentTimeStamp > app.m_renderContext.m_shaderTimestamp)
    {
        RecreateGrahpicsPipeline(app.m_renderContext);
        app.m_renderContext.m_shaderTimestamp = currentTimeStamp;
        }
    }
    
    uint32 imageIndex;
    VkResult result = vkAcquireNextImageKHR(context.m_device,
                                            context.m_swapChain,
                                            UINT64_MAX,
                                            context.m_imageAvailableSemaphores[context.m_currentFrame],
                                            VK_NULL_HANDLE,
                                            &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain(app.m_window, context);
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        SM_ASSERT(false, "failed to acquire swap chain image!");
    }
    
    vkResetFences(context.m_device, 1, &context.m_inFlightFences[context.m_currentFrame]);
    
    vkResetCommandBuffer(context.m_commandBuffers[context.m_currentFrame], 0);
    
    RecordCommandBuffer(context, renderData, imageIndex);
    UpdateUniformBuffer(context, renderData);
    
    // Update and Render additional Platform Windows
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
    
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = { context.m_imageAvailableSemaphores[context.m_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = ArrayCount(waitSemaphores);
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &context.m_commandBuffers[context.m_currentFrame];
    
    VkSemaphore signalSemaphores[] = { context.m_renderFinishedSemaphores[context.m_currentFrame] };
    submitInfo.signalSemaphoreCount = ArrayCount(signalSemaphores);
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    if (vkQueueSubmit(context.m_graphicsQueue, 1, &submitInfo, context.m_inFlightFences[context.m_currentFrame]) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to submit draw command buffer!");
    }
    
    
    
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = ArrayCount(signalSemaphores);
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapChains[] = { context.m_swapChain };
    presentInfo.swapchainCount = ArrayCount(swapChains);
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional
    
    
    
    result = vkQueuePresentKHR(context.m_presentQueue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || app.m_framebufferResized)
    {
        app.m_framebufferResized = false;
        RecreateSwapChain(app.m_window, context);
    }
    else if (result != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to present swap chain image!");
    }
    
    
    context.m_currentFrame = (context.m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

internal void CleanUpVulkan(VulkanContext & context)
{
    CleanupSwapChain(context);
    vkDestroySampler(context.m_device, context.m_textureSampler, nullptr);
    
    for (auto & [id, textureContext] : context.m_textureContexts)
    {
        vkDestroyImageView(context.m_device, textureContext.m_textureImageView, nullptr);
        vkDestroyImage(context.m_device, textureContext.m_textureImage, nullptr);
        vkFreeMemory(context.m_device, textureContext.m_textureImageMemory, nullptr);
    }
    for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(context.m_device, context.m_uniformBuffers[i], nullptr);
        vkFreeMemory(context.m_device, context.m_uniformBuffersMemory[i], nullptr);
    }
    
    vkDestroyDescriptorPool(context.m_device, context.m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(context.m_device, context.m_descriptorSetLayout, nullptr);
    
    for (auto & [id, modelContext] : context.m_modelContexts)
    {
    vkDestroyBuffer(context.m_device, modelContext.m_vertexBuffer, nullptr);
        vkFreeMemory(context.m_device, modelContext.m_vertexBufferMemory, nullptr);
        vkDestroyBuffer(context.m_device, modelContext.m_indexBuffer, nullptr);
        vkFreeMemory(context.m_device, modelContext.m_indexBufferMemory, nullptr);
    }
    
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(context.m_device, context.m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(context.m_device, context.m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(context.m_device, context.m_inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(context.m_device, context.m_commandPool, nullptr);
    vkDestroyPipeline(context.m_device, context.m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(context.m_device, context.m_pipelineLayout, nullptr);
    vkDestroyRenderPass(context.m_device, context.m_renderPass, nullptr);
    vkDestroyDevice(context.m_device, nullptr);
    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(context.m_instance, context.m_debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(context.m_instance, context.m_surface, nullptr);
    vkDestroyInstance(context.m_instance, nullptr);
    
}