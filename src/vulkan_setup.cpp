/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

internal void FramebufferResizeCallback(GLFWwindow  * window, int32 width, int32 height)
{
    Application * app = (Application *)glfwGetWindowUserPointer(window);
    app->m_framebufferResized = true;
}

internal VkVertexInputBindingDescription GetVertexBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription = {};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    return bindingDescription;
}

internal Array<VkVertexInputAttributeDescription, 2> GetVertexAttributeDescriptions()
{
    Array<VkVertexInputAttributeDescription, 2> attributeDescriptions;

    VkVertexInputAttributeDescription att1 = {};
    att1.binding = 0;
    att1.location = 0;
    att1.format = VK_FORMAT_R32G32_SFLOAT;
    att1.offset = offsetof(Vertex, m_pos);
    attributeDescriptions.Add(att1);

    VkVertexInputAttributeDescription att2 = {};
    att2.binding = 0;
    att2.location = 1;
    att2.format = VK_FORMAT_R32G32B32_SFLOAT;
    att2.offset = offsetof(Vertex, m_color);
    attributeDescriptions.Add(att2);

    return attributeDescriptions;
}

internal QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice & device, const VkSurfaceKHR  & surface)
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

internal SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice & physicalDevice, const VkSurfaceKHR & surface)
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
        //SM_ASSERT(false, "Bad Message!");
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
    
    for (const auto & format : availableFormats)    
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
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

internal VkExtent2D ChooseSwapExtent(GLFWwindow * window, const VkSurfaceCapabilitiesKHR & capabilities)
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

internal bool CheckDeviceExtensionSupport(const VkPhysicalDevice & physicalDevice)
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

internal VkDevice CreateLogicalDevice(VkPhysicalDevice & physicalDevice, VkSurfaceKHR & surface)
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
    for (int32 i = 0; i < uniqueQueueFamilies.count; i++)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
    
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.Add(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

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

internal VkQueue CreateGraphicsQueue(VkDevice & device, VkPhysicalDevice & physicalDevice, VkSurfaceKHR & surface)
{
    
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);

    VkQueue graphicsQueue;
    vkGetDeviceQueue(device, indices.m_graphicsFamily.value(), 0, &graphicsQueue);

    return graphicsQueue;
}

internal VkQueue CreatePresentQueue(VkDevice & device, VkPhysicalDevice & physicalDevice, VkSurfaceKHR & surface)
{
    
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);

    VkQueue presentQueue;
    vkGetDeviceQueue(device, indices.m_presentFamily.value(), 0, &presentQueue);

    return presentQueue;
    
}

internal VkSurfaceKHR CreateSurface(GLFWwindow * window, VkInstance & instance)
{
    
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        SM_ASSERT(false, "[SURFACE] Failed to create window surface");
    }

    return surface;
}

internal CreateSwapChainResult CreateSwapChain(GLFWwindow * window, VkDevice & device, VkPhysicalDevice & physicalDevice, VkSurfaceKHR & surface)
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

internal std::vector<VkImageView> CreateImageViews(std::vector<VkImage> & swapChainImages, VkDevice & device, VkFormat & swapChainImageFormat)
{
    std::vector<VkImageView> result(swapChainImages.size());

    for (uint32 i = 0; i < swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &result[i]) != VK_SUCCESS)
        {
            SM_ASSERT(false, "Failed to create Image view");
        }
    }

    return result;
}

// IMPORTANT: Make sure the byte code is null terminated
internal VkShaderModule CreateShaderModule(VkDevice & device, std::vector<char> & code)
{
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
                        Graphics Pipline
  ==================================================================
  https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Introduction   
  ==================================================================
 - Vertex/Index buffer -> Input Assembler  -> Vertex Shader ->
   Tessellation        -> Geomeetry shader -> Rasterization ->
   Fragment Shader     -> Color blending   -> Framebuffer
  ==================================================================
*/
internal CreateGraphicsPipelineResult
CreateGraphicsPipeline(VkDevice & device, VkExtent2D & swapChainExtent, VkRenderPass & renderPass, VkDescriptorSetLayout & descriptorSetLayout)
{
    // NOTE: This is null terminated
    std::vector<char> vertShaderCode = read_file("src/Shaders/bytecode/vert.spv");
    std::vector<char> fragShaderCode = read_file("src/Shaders/bytecode/frag.spv");

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
    Array<VkVertexInputAttributeDescription, 2> attributeDescriptions = GetVertexAttributeDescriptions();
    
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

    // alpha blending    
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

    // NOTE: Pipeline Layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;            
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

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
    pipelineInfo.pDepthStencilState = nullptr; // Optional
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

internal VkRenderPass CreateRenderPass(VkDevice & device, VkFormat & imageFormat)
{
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

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

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
CreateFramebuffers(VkDevice & device, std::vector<VkImageView> & imageViews, VkRenderPass & renderPass, VkExtent2D & extent)
{
    std::vector<VkFramebuffer> framebuffers(imageViews.size());

    for (int i = 0; i < imageViews.size(); i++)
    {
        VkImageView attachments[] = { imageViews[i] };

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

internal VkCommandPool CreateCommandPool(VkDevice & device, VkPhysicalDevice & physicalDevice, VkSurfaceKHR & surface)
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
CreateCommandBuffers(VkDevice & device, VkCommandPool & commandPool)
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

internal SyncObjects
CreateSyncObjects(VkDevice & device)
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

internal void CleanupSwapChain(Application & app)
{
    for (uint32 i = 0; i < app.m_swapChainFramebuffers.size(); i++)
    {
        vkDestroyFramebuffer(app.m_device, app.m_swapChainFramebuffers[i], nullptr);
    }

    for (uint32 i = 0; i < app.m_swapChainImageViews.size(); i++)
    {
        vkDestroyImageView(app.m_device, app.m_swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(app.m_device, app.m_swapChain, nullptr);
    
}

internal uint32 FindMemoryType(VkPhysicalDevice & physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties)
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
CreateBuffer(VkDevice & device,
             VkPhysicalDevice & physicalDevice,
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

internal void CopyBuffer(VkDevice & device,
                         VkCommandPool & commandPool,
                         VkQueue & graphicsQueue,
                         VkBuffer srcBuffer,
                         VkBuffer dstBuffer,
                         VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

internal BufferCreateResult
CreateAndBindVertexBuffer(VkDevice & device,
                          VkCommandPool & commandPool,
                          VkQueue & graphicsQueue,
                          VkPhysicalDevice & physicalDevice)
{
    VkDeviceSize bufferSize = sizeof(Vertex) * ArrayCount(vertices);
    BufferCreateResult staginBufferResult = CreateBuffer(device,
                                                          physicalDevice,
                                                          bufferSize,
                                                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    void * data;
    vkMapMemory(device, staginBufferResult.m_bufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (uint32)bufferSize);
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
CreateAndBindIndexBuffer(VkDevice & device,
                          VkCommandPool & commandPool,
                          VkQueue & graphicsQueue,
                          VkPhysicalDevice & physicalDevice)
{
    VkDeviceSize bufferSize = sizeof(vertexIndices[0]) * ArrayCount(vertexIndices);
    BufferCreateResult staginBufferResult = CreateBuffer(device,
                                                          physicalDevice,
                                                          bufferSize,
                                                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    void * data;
    vkMapMemory(device, staginBufferResult.m_bufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertexIndices, (uint32)bufferSize);
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
CreateUniformBuffers(VkDevice & device, VkPhysicalDevice & physicalDevice)
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

internal VkDescriptorSetLayout CreateDescriptiorSetLayout(VkDevice & device)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  // only vertex shader be reference to the ubo
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional relevant for image sampling related descriptors

    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = 1;
    createInfo.pBindings = &uboLayoutBinding;

    VkDescriptorSetLayout result;
    if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &result) != VK_SUCCESS)
    {
        SM_ASSERT(false, "faile to create descriptor set layout!");
    }

    return result;
}

internal VkDescriptorPool CreateDescriptorPool(VkDevice & device)
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = (uint32)MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = (uint32)MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPool pool;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to create descriptor pool!");
    }

    return pool;
    
}

internal Array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>
CreateDescriptorSets(VkDevice & device,
                     Array<VkBuffer, MAX_FRAMES_IN_FLIGHT> uniformBuffers,
                     VkDescriptorPool & descriptorPool,
                     VkDescriptorSetLayout & descriptorSetLayout)
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

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;

        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;

        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    return descriptorSets;
}

internal IsDeviceSuitableResult IsDeviceSuitable(const VkPhysicalDevice & device, const VkSurfaceKHR & surface)
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
    
    result.m_isSuitable = indices.IsComplete() && extensionsSupported && swapChainAdequate;
    
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

internal VkPhysicalDevice PickPhysicalDevice(VkInstance & instance, VkSurfaceKHR & surface)
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

    VkClearValue clearColor =  {{{0.0f, 0.0f, 0.0f, 1.0f}}};
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
    
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to record command buffer!");
    }
}

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

internal void UpdateUniformBuffer(Array<void *, MAX_FRAMES_IN_FLIGHT> uniformBuffersMapped,
                                  VkExtent2D & imageExent,
                                  uint32 currentFrame)
{
    local_persist auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    real32 timeElapsed = std::chrono::duration<real32, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo = {};
    ubo.m_model      = glm::rotate(glm::mat4(1.0f), timeElapsed * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.m_view       = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.m_projection = glm::perspective(glm::radians(45.0f), imageExent.width / (real32) imageExent.height, 0.1f, 10.0f);
    ubo.m_projection[1][1] *= -1;

    memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
    
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

    UpdateUniformBuffer(app.m_uniformBuffersMapped, app.m_swapChainExtent, app.m_currentFrame);

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
