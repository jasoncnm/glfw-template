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

struct MouseButton
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
    glm::vec2 prevMousePos = glm::vec2(0);
    glm::vec2 mousePos = glm::vec2(0);
    
    real32 mouseScrollDelta = 0.0f;
    
    // NOTE: see key mappings https://www.glfw.org/docs/3.3/group__keys.html
    Key keys[GLFW_KEY_LAST + 1];
    // NOTE: see mouse button mappings:
    // https://www.glfw.org/docs/3.3/group__buttons.html#ga3e2f2cf3c4942df73cc094247d275e74
    MouseButton mouseButtons[GLFW_MOUSE_BUTTON_MIDDLE];
};

//  ========================================================================
//              NOTE: Input Functions
//  ========================================================================

bool KeyIsDown(Input & input, uint32 code)
{
    return input.keys[code].isDown;
}

bool MouseButtonDown(Input & input, uint32 code)
{
    return input.mouseButtons[code].isDown;
}

#define INPUT_H
#endif
