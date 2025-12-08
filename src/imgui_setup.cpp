/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "imgui_internal.h"

internal QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR  surface);

internal void InitImGui(Application & app)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
     io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
     // ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    ImGui::StyleColorsClassic();
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.0f);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = 1.5f;
    style.FrameRounding = 4;
    io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(app.m_window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.ApiVersion = VK_API_VERSION_1_0;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
    init_info.Instance = app.m_renderContext.m_instance;
    init_info.PhysicalDevice = app.m_renderContext.m_physicalDevice;
    init_info.Device = app.m_renderContext.m_device;

    QueueFamilyIndices indices = FindQueueFamilies(app.m_renderContext.m_physicalDevice, app.m_renderContext.m_surface);
    init_info.QueueFamily = indices.m_graphicsFamily.value();
    init_info.Queue = app.m_renderContext.m_graphicsQueue;

    init_info.PipelineCache = VK_NULL_HANDLE;
    
    init_info.DescriptorPool = app.m_renderContext.m_imGuiDescriptorPool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.PipelineInfoMain.RenderPass = app.m_renderContext.m_imGuiRenderPass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    init_info.CheckVkResultFn =
        [](VkResult result)
        {
            SM_ASSERT(result == VK_SUCCESS, "failed to initialize vulkan imgui! error code %d", result);
        };
    
    
    ImGui_ImplVulkan_Init(&init_info);
    
    VulkanContext & context = app.m_renderContext;
    context.m_imguiDset = ImGui_ImplVulkan_AddTexture(context.m_textureSampler, context.m_textureContexts[app.m_renderData.m_transforms[0].m_textureID].m_textureImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
}

internal void CleanUpImgui()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

internal void UpdateImGui(Application & app)
{
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    ImGuiID dockspace_id = ImGui::GetID("My Dockspace");
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    // Create settings
    if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
    {
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
        ImGuiID dock_id_left = 0;
        ImGuiID dock_id_main = dockspace_id;
        ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.20f, &dock_id_left, &dock_id_main);
        ImGuiID dock_id_left_top = 0;
        ImGuiID dock_id_left_bottom = 0;
        ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Up, 0.50f, &dock_id_left_top, &dock_id_left_bottom);
        ImGui::DockBuilderDockWindow("Game", dock_id_main);
        ImGui::DockBuilderDockWindow("Properties", dock_id_left_top);
        ImGui::DockBuilderDockWindow("Scene", dock_id_left_bottom);
        ImGui::DockBuilderFinish(dockspace_id);
    }
    
    // Submit dockspace
    ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode);
    
    int32 gx, gy, gw, gh;
    // Submit Windows
    ImGui::Begin("Game");
    
    ImVec2 pos = ImGui::GetWindowPos();
    gx = (int32)pos.x;
    gy = (int32)pos.y;
    
    ImVec2 size = ImGui::GetWindowSize();
     gw = (int32)size.x;
    gh = (int32)size.y;
    
    app.m_renderData.m_renderWindowRect = { {gx, gy}, {gw, gh} };
    
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    ImGui::Image(app.m_renderContext.m_imguiDset, ImVec2{viewportPanelSize.x, viewportPanelSize.y});
    
    ImGui::End();
    
    ImGui::Begin("Properties");
    
    ImGui::Text("Window Position (%d, %d)", gx, gy);
    ImGui::Text("Window Size (%d, %d)", gw, gh);
    
    ImGui::End();
    
    ImGui::Begin("Scene");
    
    ImGui::End();
    
    
    local_persist bool show_another_window = false, show_debug_window = false;
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if (ImGui::BeginMainMenuBar())
    {
        
    if (ImGui::BeginMenu("Application"))
        {
            if (ImGui::MenuItem("Quit", "Q"))
            {
                app.m_running = false;
            }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit"))
        {
            if (!show_debug_window && ImGui::MenuItem("Show Debug Window"))
            {
                show_debug_window = true;
            }
            else if (show_debug_window && ImGui::MenuItem("Close Debug Window"))
            {
                show_debug_window = false;
            }
            ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
    }
    app.m_editingImgui = io.WantCaptureMouse;
    
    if (show_debug_window) 
    {
        
        static int counter = 0;

        ImGui::Begin("Hello, world!", &show_debug_window); // Create a window called "Hello, world!" and append into it.
        
        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Another Window", &show_another_window);
        
        Camera & camera = app.m_renderData.m_camera;
        
        for (uint32 i  = 0; i < app.m_renderData.m_transforms.count; i++)
        {
            ImGui::PushID(i);
            Transform & tr = app.m_renderData.m_transforms[i];
            char text[200];
            sprintf(text, "Num mesh instances, %d", i);
            ImGui::SliderInt(text, (int *)&tr.m_numCopies, 1, 5000);
            ImGui::PopID();
        }

        ImGui::SliderFloat("Fog Distence", &app.m_renderData.m_fog.m_viewDistence, 1.0f, 50.0f);
        ImGui::SliderFloat("Fog Steepness", &app.m_renderData.m_fog.m_steepness, 0.0f, 10.0f);
        ImGui::SliderFloat("camera fov", &camera.m_fovy, 10.0f, 100.0f);
        ImGui::SliderFloat("near plane", &camera.m_nearClip, 0.1f, 10.0f);
        ImGui::SliderFloat("far plane", &camera.m_farClip, 10.0f, 100.0f);
        ImGui::ColorEdit3("clear color", (float*)&app.m_renderData.m_clearColor); // Edit 3 floats representing a color
        ImGui::ColorEdit3("fog color", (float*)&app.m_renderData.m_fog.m_fogColor); // Edit 3 floats representing a color
        
        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        {
            counter++;
        }
        
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);
        
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Text("Camera Pitch %.2f, Camera Yaw %.2f", 
                    camera.m_pitch,
                    camera.m_yaw);
        ImGui::Text("Camera Position (%.2f, %.2f, %.2f)", 
                    camera.m_pos.x,
                    camera.m_pos.y,
                    camera.m_pos.z);
        
        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
        
        ImGui::ShowDemoWindow();
        
        ImGui::End();
}
    
    }
