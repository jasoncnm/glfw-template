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


//====================================================
//      NOTE: Application Structs
//====================================================

struct Application
{
    GLFWwindow * m_window;
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;
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
