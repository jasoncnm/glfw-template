/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

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
