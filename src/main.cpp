/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "application.cpp"

int main()
{
    Application * app = new(Application);
    if (!app)
    {
        SM_ERROR("Failed to allocate application");
        return -1;
    }
    
    RunApplication(app);
    delete app;
}
