#if !defined(INPUT_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

// TODO: implement

struct Key
{
    b8 isDown;
    b8 isJustPressed;
    b8 isJustReleased;
    uint8 halfTransitionCount;
};

struct Input
{
    IVec2 screenSize;

    // NOTE: Screen
    IVec2 prevMousePos;
    IVec2 mousePos;
    IVec2 relMouse;

    // NOTE: World
    IVec2 prevMousePosWorld;
    IVec2 mousePosWorld;
    IVec2 relMouseWorld;

}

#define INPUT_H
#endif
