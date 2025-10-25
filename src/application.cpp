/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
#include "application.h"

//====================================================
//      NOTE: Application Functions
//====================================================
QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice & device, const VkSurfaceKHR  & surface)
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

SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice & physicalDevice, const VkSurfaceKHR & surface)
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

VkResult CreateDebugUtilsMessengerEXT(
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

void DestroyDebugUtilsMessengerEXT(
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

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback (
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

VkDebugUtilsMessengerCreateInfoEXT
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

void SetupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT * pdebugMessenger)
{
    SM_TRACE("[DEBUG_MSGER] Seting up Debug Messenger");
    
    if (!enableValidationLayers) return;

    auto createInfo = GetDebugMessengerCreateInfo();

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, pdebugMessenger) != VK_SUCCESS)
    {
        SM_ASSERT(false, "[DEBUG_MSGER] failed to set up debug messenger!");
    }
    
    SM_TRACE("[DEBUG_MSGER] Debug Messenger setup Successfully");
}

bool CheckValidationLayerSupport()
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

std::vector<const char *> GetRequiredExtensions()
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

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> & availableFormats)
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

VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> & availablePresentModes)
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

VkExtent2D ChooseSwapExtent(GLFWwindow * window, const VkSurfaceCapabilitiesKHR & capabilities)
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

VkInstance CreateVkInstance()
{
    SM_TRACE("[INSTANCE] Creating Vulkan Instance");

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

    uint32 glfwExtensionCount = 0;
    const char ** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

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
    
    SM_TRACE("[INSTANCE] Vulkan Instance Create Successfully!");

    return instance;
}

bool CheckDeviceExtensionSupport(const VkPhysicalDevice & physicalDevice)
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

bool IsDeviceSuitable(const VkPhysicalDevice & device, const VkSurfaceKHR & surface)
{
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
    
    bool result = indices.IsComplete() && extensionsSupported && swapChainAdequate;
    
    /*
      ======================================== IMPORTANT===========================================
      Current implementation only support PC with discrete GPU 
      =============================================================================================
      Instead of just checking if a device is suitable or not and going with the first one,
      you could also give each device a score and pick the highest one.
      That way you could favor a dedicated graphics card by giving it a higher score,
      but fall back to an integrated GPU if that's the only available one. 
      =============================================================================================
    */
    
    result = result && deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

    if (result)
    {
        SM_TRACE("[PHYSICAL_DEVICE] [SELETED_DEVICE] %s", deviceProperties.deviceName); 
    }

    return result;
}

VkPhysicalDevice PickPhysicalDevice(VkInstance & instance, VkSurfaceKHR & surface)
{
    SM_TRACE("[PHYSICAL_DEVICE] Creating Physical Device");

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    uint32 deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (!deviceCount)
    {
        SM_ASSERT(false, "[PHYSICAL_DEVICE] failed to find GPUs with Vulkan support!");        
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto & device : devices)
    {
        if (IsDeviceSuitable(device, surface))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        SM_ASSERT(false, "[PHYSICAL_DEVICE] failed to find a suitable GPU!");        
    }

    SM_TRACE("[PHYSICAL_DEVICE] Physical Device Create Successfully!");
    return physicalDevice;
}


VkDevice CreateLogicalDevice(VkPhysicalDevice & physicalDevice, VkSurfaceKHR & surface)
{
    SM_TRACE("[DEVICE] Creating Logical Device");

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

    SM_TRACE("[DEVICE] Logical Device Create Successfully!");

    return device;    
}

VkQueue CreateGraphicsQueue(VkDevice & device, VkPhysicalDevice & physicalDevice, VkSurfaceKHR & surface)
{
    SM_TRACE("[GRAPHICS_QUEUE] Creating Graphics Queue");
    
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);

    VkQueue graphicsQueue;
    vkGetDeviceQueue(device, indices.m_graphicsFamily.value(), 0, &graphicsQueue);

    SM_TRACE("[GRAPHICS_QUEUE] Graphics Queue Create Successfully!");
    return graphicsQueue;
}

VkQueue CreatePresentQueue(VkDevice & device, VkPhysicalDevice & physicalDevice, VkSurfaceKHR & surface)
{
    SM_TRACE("[PRESENT_QUEUE] Creating Present Queue");
    
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);

    VkQueue presentQueue;
    vkGetDeviceQueue(device, indices.m_presentFamily.value(), 0, &presentQueue);

    SM_TRACE("[PRESENT_QUEUE] Present Queue Create Successfully!");
    return presentQueue;
    
}

VkSurfaceKHR CreateSurface(GLFWwindow * window, VkInstance & instance)
{
    SM_TRACE("[SURFACE] Creating Window Surface");
    
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        SM_ASSERT(false, "[SURFACE] Failed to create window surface");
    }

    SM_TRACE("[SURFACE] Window Surface Create Successfully!");
    return surface;
}

CreateSwapChainResult CreateSwapChain(GLFWwindow * window, VkDevice & device, VkPhysicalDevice & physicalDevice, VkSurfaceKHR & surface)
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
    
    VkSwapchainKHR swapChain;
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        SM_ASSERT(false, "Failed to create swap chain!");
    }
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);

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

std::vector<VkImageView> CreateImageViews(std::vector<VkImage> & swapChainImages, VkDevice & device, VkFormat & swapChainImageFormat)
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

void InitVulkan(Application & app)
{
    app.m_instance = CreateVkInstance();
    SetupDebugMessenger(app.m_instance, &app.m_debugMessenger);
    app.m_surface = CreateSurface(app.m_window, app.m_instance);
    app.m_physicalDevice = PickPhysicalDevice(app.m_instance, app.m_surface);
    app.m_device = CreateLogicalDevice(app.m_physicalDevice, app.m_surface);
    app.m_graphicsQueue = CreateGraphicsQueue(app.m_device, app.m_physicalDevice, app.m_surface);
    app.m_presentQueue = CreatePresentQueue(app.m_device, app.m_physicalDevice, app.m_surface);

    // NOTE: swapchain, images, format, extent creation
    {
        CreateSwapChainResult createResult = CreateSwapChain(app.m_window, app.m_device, app.m_physicalDevice, app.m_surface);
        app.m_swapChain = createResult.m_swapChain;
        app.m_swapChainImages = createResult.m_swapChainImages;
        app.m_swapChainImageFormat = createResult.m_swapChainImageFormat;
        app.m_swapChainExtent = createResult.m_swapChainExtent;
    }

    app.m_swapChainImageViews = CreateImageViews(app.m_swapChainImages, app.m_device, app.m_swapChainImageFormat);
    
}



void InitWindow(Application & app)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    app.m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void CleanUp(Application & app)
{
    for (uint32 i = 0; i < app.m_swapChainImageViews.size(); i++)
    {
        vkDestroyImageView(app.m_device, app.m_swapChainImageViews[i], nullptr);
    }
    vkDestroySwapchainKHR(app.m_device, app.m_swapChain, nullptr);
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
void MainLoop(Application & app)
{
    for ( ;!glfwWindowShouldClose(app.m_window); )
    {
        glfwPollEvents();
    }
}
