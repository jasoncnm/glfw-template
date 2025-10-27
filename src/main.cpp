/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "engine_lib.h"
#include "application.cpp"

internal void Run(Application & app)
{
    // NOTE: Run Application
    InitWindow(app);
    InitVulkan(app);
    MainLoop(app);
    CleanUp(app);
}

int main()
{
    Application app;
    Run(app);

}
