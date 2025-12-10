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
    BumpAllocator transientStorage = MakeBumpAllocator(MB(64));
    BumpAllocator persistentStorage = MakeBumpAllocator(MB(64));
    Application * app = (Application *)BumpAlloc(&persistentStorage, sizeof(Application));
    if (!app)
    {
        SM_ERROR("Failed to allocate application");
        return -1;
    }
    
    RunApplication(app);
}
