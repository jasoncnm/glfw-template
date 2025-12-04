  /* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
#include "application.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include <glm/gtx/quaternion.hpp>

/*
TODO: Things that I can do
- Continue mipmaping tutorial
    - Material System
- Normal mapping
- Skybox rendering
- Add lighting
- PBR
  - hierarchical model
- Ambient occulsion
- Current We can only Render one transform. 
    - Draw shader arts
- Support passing multiple textures to fragment shader
- Impliment custom hash map without using std::hash and std::unordered_map
   - Impliment custom arena allocator for vulkan object allocations
  */

//====================================================
//      NOTE: Application Functions
//====================================================
#include "imgui_setup.cpp"
#include "vulkan_backend.cpp"

internal glm::vec3 HexToRGB(uint32 val)
{
    uint32 r = ( val >> 16 ) & 0xFF;
    uint32 g = ( val >> 8  ) & 0xFF;
    uint32 b = ( val >> 0  ) & 0xFF;
    
    return glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);    
}

internal uint32 VertexHash(const Vertex & vertex)
{
    uint32 h1 = (uint32)std::hash<glm::vec3>()(vertex.m_pos);
    uint32 h2 = (uint32)std::hash<glm::vec3>()(vertex.m_color);
    uint32 h3 = (uint32)std::hash<glm::vec2>()(vertex.m_texCoord);
    
    return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1);
}

template<> struct std::hash<Vertex>
{
    size_t operator()(Vertex const& vertex) const
    {
        return VertexHash(vertex);
    }
};

internal Model LoadModel(const char * objFileName)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;
    
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objFileName);
    if (!warn.empty())
    {
        SM_WARN("%s", warn.c_str());
    }
    if (!err.empty())
    {
        SM_ERROR("%s", err.c_str());
    }
    
    SM_ASSERT(ret, "failed to load object file, err: %s", err.c_str());
    
    std::unordered_map<Vertex, uint32> uniqueVertices = {};
    Model model = {};
    
    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++)
    {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            
            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++)
            {
                Vertex vertex = {};
                
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                
                vertex.m_pos.x = attrib.vertices[3*size_t(idx.vertex_index)+0];
                vertex.m_pos.y = attrib.vertices[3*size_t(idx.vertex_index)+1];
                vertex.m_pos.z = attrib.vertices[3*size_t(idx.vertex_index)+2];
                
                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
                    tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
                    tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];
                }
                
                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    vertex.m_texCoord.x = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
                    vertex.m_texCoord.y = 1.0f - attrib.texcoords[2*size_t(idx.texcoord_index)+1];
                    //vertex.m_texCoord.y = attrib.texcoords[2*size_t(idx.texcoord_index)+1];
                }
                
                vertex.m_color = { 1.0f, 1.0f, 1.0f };
                
                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = (uint32)model.m_vertices.size();
                    model.m_vertices.push_back(vertex);
                }
                
                model.m_indices.push_back(uniqueVertices[vertex]);
            }
            
            index_offset += fv;
            // per-face material
            // shapes[s].mesh.material_ids[f];
        }
    }
    
    size_t vertexSize = model.m_vertices.size();
    
    return model;
}

void InitRenderData(Application & app)
{
    
    Transform tr;
    tr.m_meshPositions = {
        glm::vec3( 0.0f,  0.0f,  0.0f), 
        glm::vec3( 2.0f,  5.0f, -15.0f), 
        glm::vec3(-1.5f, -2.2f, -2.5f),  
        glm::vec3(-3.8f, -2.0f, -12.3f),  
        glm::vec3( 2.4f, -0.4f, -3.5f),  
        glm::vec3(-1.7f,  3.0f, -7.5f),  
        glm::vec3( 1.3f, -2.0f, -2.5f),  
        glm::vec3( 1.5f,  2.0f, -2.5f), 
        glm::vec3( 1.5f,  0.2f, -1.5f), 
        glm::vec3(-1.3f,  1.0f, -1.5f)  
    };
    
    tr.m_model = LoadModel(MODEL_PATH);
    
    app.m_renderData.m_transform = tr;
    app.m_renderData.m_camera = {};
    app.m_renderData.m_camera.m_pos = { 8.78f, -11.88f, 5.66f };
    app.m_renderData.m_camera.m_pitch = -36;
    app.m_renderData.m_camera.m_yaw = -31;
    app.m_renderData.m_clearColor = glm::vec4(HexToRGB(0x142A9C), 1.0f);
}

internal void FramebufferResizeCallback(GLFWwindow  * window, int32 width, int32 height)
{
    Application * app = (Application *)glfwGetWindowUserPointer(window);
    app->m_framebufferResized = true;
}

internal void InputKeyCallback(GLFWwindow * _window, int32 keycode, int32 scancode, int32 action, int32 mods)
{
    Application * app = (Application *)glfwGetWindowUserPointer(_window);
    Key * key = &app->m_input.keys[keycode];
    
        switch(action)
    {
        case GLFW_PRESS:
        {
            key->isDown = true;
            key->halfTransitionCount++;
            break;
        }
        case GLFW_RELEASE:
        {
            key->isDown = false;
            key->halfTransitionCount++;
            break;
        }
        case GLFW_REPEAT:
        {
            // NOTE: TYPE
        }
        default: {}
    }
    }

internal void InputMouseButtonCallback(GLFWwindow * _window, int32 button, int32 action, int32 mods)
{
    Application * app = (Application *)glfwGetWindowUserPointer(_window);
     MouseButton * mouseButton = &app->m_input.mouseButtons[button];
    
    switch(action)
    {
        case GLFW_PRESS:
        {
            mouseButton->isDown = true;
            mouseButton->halfTransitionCount++;
            break;
        }
        case GLFW_RELEASE:
        {
            mouseButton->isDown = false;
            mouseButton->halfTransitionCount++;
            break;
        }
        default: {}
    }
}

internal void InputScrollCallback(GLFWwindow* _window, real64  xoffset, real64 yoffset)
{
    SM_TRACE("MouseScroll: (%.1f, %.1f)", xoffset, yoffset);
    Application * app = (Application *)glfwGetWindowUserPointer(_window);
    app->m_input.mouseScrollDelta = (real32)yoffset;
    }

internal void InputMouseCursorCallback(GLFWwindow* _window, real64 xpos, real64 ypos)
{
    // SM_TRACE("mouse position: (%.2f, %.2f)", xpos, ypos);
    Application * app = (Application *)glfwGetWindowUserPointer(_window);
    app->m_input.mousePos = { xpos, ypos };
    }

internal void InputSetJoystickCallback(int32 jid, int32 event)
{
    if (event == GLFW_CONNECTED)
    {
        // The joystick was connected
        const char* name = glfwGetJoystickName(jid);
        SM_TRACE("joystick %s is connected, joystick id: %d", name, jid);
    }
    else if (event == GLFW_DISCONNECTED)
    {
        // The joystick was disconnected
        const char* name = glfwGetJoystickName(jid);
        SM_TRACE("joystick %s is disconnected, joystick id: %d", name, jid);
    }
}

internal void InitInput(Application & app)
{
    app.m_input = {};
    
    app.m_input.screenSize = { WIDTH, HEIGHT };
    
    for (uint32 i = 0; i <= GLFW_KEY_LAST; i++) 
    {
        app.m_input.keys[i] = {};
    }
}

internal void InitWindow(Application & app)
{
    SM_ASSERT(glfwInit(), "Could not initilize GLFW!");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    app.m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    SM_ASSERT(app.m_window, "Could not create window");
    SM_ASSERT(glfwVulkanSupported(), "GLFW: Vulkan Not Supported\n");


    GLFWmonitor * primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode * videoMode = glfwGetVideoMode(primaryMonitor);
    int32 windowLeft = videoMode->width / 2 - WIDTH / 2;
    int32 windowTop = videoMode->height / 2 - HEIGHT / 2;
    glfwSetWindowPos(app.m_window, windowLeft, windowTop);

    glfwSetWindowUserPointer(app.m_window, &app);
    glfwSetFramebufferSizeCallback(app.m_window, FramebufferResizeCallback);

    // NOTE: SetVSync;
    glfwSwapInterval(1);
    app.m_vSync = true;

    // NOTE: Set MouseInputOptions
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(app.m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // NOTE: Set GLFW callbacks
    glfwSetKeyCallback(app.m_window, InputKeyCallback);
glfwSetMouseButtonCallback(app.m_window, InputMouseButtonCallback);
glfwSetScrollCallback(app.m_window, InputScrollCallback);
glfwSetCursorPosCallback(app.m_window, InputMouseCursorCallback);
glfwSetJoystickCallback(InputSetJoystickCallback);
    
    app.m_running = true;
    glfwSetTime(0.0);
}

internal void Update(Application & app, float dt)
{
    Input & input = app.m_input;
    
    if (KeyIsDown(input, GLFW_KEY_Q))
    {
        app.m_running = false;
    }
    
    real32 cameraMoveSpeed = 5.0f;
    real32 pitchSpeed = 2.0f * 50.0f;
    real32 yawSpeed = 2.0f * 50.0f;
    Camera & camera = app.m_renderData.m_camera;
    
    if (Abs(input.mouseScrollDelta) > 0)
    {
        real32 step = 3.0f;
        camera.m_fovy -= step * input.mouseScrollDelta;
        if (camera.m_fovy > 100.0f) camera.m_fovy = 95.0f;
        if (camera.m_fovy < 1.0f) camera.m_fovy = 1.0f;
    }
    
    glm::vec3 forward = camera.m_forwardDirection;
    glm::vec3 right   = glm::cross(forward, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::vec3 up      = camera.m_up;
    
    if (MouseButtonDown(input, GLFW_MOUSE_BUTTON_RIGHT))
    {
        real32 sensitivity = 0.1f;
        glm::vec2 offset = input.mousePos - input.prevMousePos;
        offset *= sensitivity;
        camera.m_yaw += offset.x;
        camera.m_pitch += -offset.y;
    }
    else
    {
    if (KeyIsDown(input, GLFW_KEY_UP))
    {
            camera.m_pitch += pitchSpeed * dt; 
    }
    
    if (KeyIsDown(input, GLFW_KEY_DOWN))
    {
            camera.m_pitch += -pitchSpeed * dt;
    }
    
    if (KeyIsDown(input, GLFW_KEY_LEFT))
    {
            camera.m_yaw += -yawSpeed * dt;
    }
    
    if (KeyIsDown(input, GLFW_KEY_RIGHT))
    {
            camera.m_yaw += yawSpeed * dt;
    }
}
    
    if(camera.m_pitch > 89.0f)
        camera.m_pitch =  89.0f;
    if(camera.m_pitch < -89.0f)
        camera.m_pitch = -89.0f;
    
    glm::quat pitchQuat = glm::angleAxis(glm::radians(camera.m_pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yawQuat = glm::angleAxis(-glm::radians(camera.m_yaw), glm::vec3(0.0f, 0.0f, 1.0f));
    camera.m_forwardDirection = glm::rotate(yawQuat * pitchQuat, glm::vec3(0.0f, 1.0f, 0.0f));

    glm::vec3 moveDir(0);
    
    forward = camera.m_forwardDirection;
    right   = glm::cross(forward, glm::vec3(0.0f, 0.0f, 1.0f));
    up      = glm::cross(right, forward);
    
    if (KeyIsDown(input, GLFW_KEY_W)) 
    {
        moveDir += forward;
    }
    
    if (KeyIsDown(input, GLFW_KEY_S)) 
    {
        moveDir += -forward; 
    }
    
    if (KeyIsDown(input, GLFW_KEY_A)) 
    {
        moveDir += -right; 
        }
    
    if (KeyIsDown(input, GLFW_KEY_D)) 
    {
        moveDir += right; 
    }
    
    if (KeyIsDown(input, GLFW_KEY_LEFT_SHIFT) || KeyIsDown(input, GLFW_KEY_SPACE))
    {
        moveDir += up;
    }
    
    if (KeyIsDown(input, GLFW_KEY_LEFT_CONTROL))
    {
        moveDir += -up;
    }
    
    if (moveDir != glm::vec3(0))
    {
        camera.m_pos += glm::normalize(moveDir) * cameraMoveSpeed * dt;
    }
    
    camera.m_up = up;
    
}

internal void Init(Application & app)
{
    InitWindow(app);
    InitInput(app);
    InitRenderData(app);
    InitVulkan(app);
    InitImGui(app);
    }

internal void CleanUp(Application & app)
{
    CleanUpImgui();
    CleanUpVulkan(app.m_renderContext);
    glfwDestroyWindow(app.m_window);
    glfwTerminate();
}

internal void MainLoop(Application & app)
{
    real64 currentTime = glfwGetTime();
    for ( ;app.m_running; )
    {
        real64 time = glfwGetTime();
        real32 dt = (real32)(time - currentTime);
        currentTime = time;
        
        glfwPollEvents();
        
        ImguiStartFrame(app);
        
        Update(app, dt);

        // Rendering
        DrawFrame(app, &app.m_renderData);
        app.m_running = app.m_running && !glfwWindowShouldClose(app.m_window);
        
        // NOTE: Reset/Update Input End of frame
        app.m_input.mouseScrollDelta = 0.0f;
        app.m_input.prevMousePos = app.m_input.mousePos;
        
    }
    vkDeviceWaitIdle(app.m_renderContext.m_device);
    }

void RunApplication(Application & app)
{
    // NOTE: Run Application
    Init(app);
    MainLoop(app);
    CleanUp(app);
    }
