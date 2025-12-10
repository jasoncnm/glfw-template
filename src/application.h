#if !defined(APPLICATION_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */


#include "vulkan_backend.h"
#include "engine_lib.h"
#include "render_interface.h"
#include "input.h"

#include <chrono>
#include <unordered_map>
#include <GLFW/glfw3.h>

//====================================================
//      NOTE: Application Constexpr
//====================================================

constexpr int32 WIDTH = 1920;
constexpr int32 HEIGHT = 1080;

//====================================================
//      NOTE: Application Structs
//====================================================

struct Application
{
    int32 m_joystick = -1;
    
    bool   m_framebufferResized = false;
    bool   m_vSync = false;
    bool   m_running = false;
    bool   m_editingImgui = false;
    
    
    real32 m_fixedTimeStep = 0.1f;
    
    GLFWwindow  * m_window;
    RenderData    m_renderData;
    VulkanContext m_renderContext;
    Input         m_input;
};

//====================================================
//      NOTE: Application Functions
//====================================================
void RunApplication(Application * app);

#define APPLICATION_H
#endif
