#if !defined(INPUT_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include <GLFW/glfw3.h>

// TODO: implement

struct Key
{
    b8 isDown = false;
    b8 isJustPressed = false;
    b8 isJustReleased = false;
    uint8 halfTransitionCount = 0;
};

struct Input
{
    IVec2 screenSize;

    // NOTE: Screen
    IVec2 prevMousePos;
    IVec2 mousePos;
    IVec2 relMouse;

    Key keys[GLFW_KEY_LAST + 1];
};

//  ========================================================================
//              NOTE: Input Functions
//  ========================================================================

bool KeyIsDown(Input & input, uint32 code)
{
    return input.keys[code].isDown;
}

#define INPUT_H
#endif
