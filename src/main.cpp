/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "engine_lib.h"

#include <iostream>
#include <GLFW/glfw3.h>



int main (int argc, char * argv[])
{

    SM_TRACE("HELLO");

    if (!glfwInit())
    {
        SM_ERROR("Initializtion failed");
        return -1;
    }

    SM_TRACE("GLFW Initilized");

    glfwTerminate();

    SM_TRACE("GLFW Terminiated");
    
}
