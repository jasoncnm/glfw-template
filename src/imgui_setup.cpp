/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
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
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
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
    init_info.DescriptorPool = app.m_renderContext.m_descriptorPool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = (uint32)app.m_renderContext.m_swapChainImages.size();
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.PipelineInfoMain.RenderPass = app.m_renderContext.m_renderPass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    init_info.CheckVkResultFn =
        [](VkResult result)
        {
            SM_ASSERT(result == VK_SUCCESS, "failed to initialize vulkan imgui! error code %d", result);
        };
    
    ImGui_ImplVulkan_Init(&init_info);
}

internal void CleanUpImgui()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

internal void ImguiStartFrame(Application & app)
{
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
// Create a dockspace in main viewport, where central node is transparent.
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
    
    #if 0
    local_persist bool show_demo_window = false, show_another_window = false;
ImGuiIO& io = ImGui::GetIO(); (void)io;
if (show_demo_window)
    {
        ImGui::ShowDemoWindow(&show_demo_window);
    }
    
    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
        
        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);
        
        Camera & camera = app.m_renderData.m_camera;
        ImGui::SliderFloat("camera fov", &camera.m_fovy, 10.0f, 180.0f);
        ImGui::SliderFloat("near plane", &camera.m_nearClip, 0.1f, 10.0f);
        ImGui::SliderFloat("far plane", &camera.m_farClip, 10.0f, 100.0f);
        ImGui::ColorEdit3("clear color", (float*)&app.m_renderData.m_clearColor); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Text("Camera Forward direction (%.2f, %.2f, %.2f)", 
                    camera.m_forwardDirection.x,
                    camera.m_forwardDirection.y,
                    camera.m_forwardDirection.z);
                    
        
        Input & input = app.m_input;
        
        if (KeyIsDown(input, GLFW_KEY_W)) 
        {
            ImGui::Text("Key W is down."); 
        }
        
        if (KeyIsDown(input, GLFW_KEY_A)) 
        {
            ImGui::Text("Key A is down."); 
        }
        
        if (KeyIsDown(input, GLFW_KEY_S)) 
        {
            ImGui::Text("Key S is down."); 
        }
        
        if (KeyIsDown(input, GLFW_KEY_D)) 
        {
            ImGui::Text("Key D is down."); 
        }
        
        ImGui::End();

    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }
    #endif
}
