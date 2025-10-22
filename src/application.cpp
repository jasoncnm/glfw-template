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
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
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

        if (indices.IsComplete()) break;
    }
    
    return indices;
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

void SetupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT * pdebugMessenger) {

    if (!enableValidationLayers) return;

    auto createInfo = GetDebugMessengerCreateInfo();

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, pdebugMessenger) != VK_SUCCESS)
    {
        SM_ASSERT(false, "failed to set up debug messenger!");
    }
    
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

VkInstance CreateVkInstance()
{
    SM_TRACE("Creating Vulkan Instance");

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
        SM_ASSERT(false, "failed to create instance!");
    }
    
    SM_TRACE("Vulkan Instance Create Successfully!");

    return instance;
}

bool IsDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    QueueFamilyIndices indices = FindQueueFamilies(device);
    
    bool result = indices.IsComplete();
    
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
    
    result &= deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

    if (result)
    {
        SM_TRACE("[SELETED_DEVICE] %s", deviceProperties.deviceName); 
    }

    return result;
}

VkPhysicalDevice PickPhysicalDevice(VkInstance instance)
{
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    uint32 deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (!deviceCount)
    {
        SM_ASSERT(false, "failed to find GPUs with Vulkan support!");        
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto & device : devices)
    {
        if (IsDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        SM_ASSERT(false, "failed to find a suitable GPU!");        
    }

    return physicalDevice;
}

void InitVulkan(Application & app)
{
    app.m_instance = CreateVkInstance();

    SetupDebugMessenger(app.m_instance, &app.m_debugMessenger);

    app.m_device = PickPhysicalDevice(app.m_instance);
}



void InitWindow(Application & app)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    app.m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

// NOTE: call at each frame
void MainLoop(Application & app)
{
    for ( ;!glfwWindowShouldClose(app.m_window); )
    {
        glfwPollEvents();
    }
}

void CleanUp(Application & app)
{
    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(app.m_instance, app.m_debugMessenger, nullptr);
    }
    vkDestroyInstance(app.m_instance, nullptr);
    glfwDestroyWindow(app.m_window);
    glfwTerminate();
}
