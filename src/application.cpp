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

/*
TODO: Things that I can do
  - Basic perspective camera control
  - Impliment custom hash map without using std::hash and std::unordered_map
  - Impliment custom arena allocator for vulkan object allocations
  - Continue mipmaping tutorial
  - Draw shader arts
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
    app.m_renderData.m_model = LoadModel(MODEL_PATH);
    app.m_renderData.m_camera = {};
    app.m_renderData.m_clearColor = glm::vec4(HexToRGB(0x142A9C), 1.0f);
}

internal void FramebufferResizeCallback(GLFWwindow  * window, int32 width, int32 height)
{
    Application * app = (Application *)glfwGetWindowUserPointer(window);
    app->m_framebufferResized = true;
}

internal void InputKeyCallback(GLFWwindow * _window, int32 key, int32 scancode, int32 action, int32 mods)
{
    // Application * app = (Application *)glfwGetWindowUserPointer(_window);
    
    switch(action)
    {
        case GLFW_PRESS:
        {
            const char * keyname = glfwGetKeyName(key, scancode);
            if (keyname)
            {
                SM_TRACE("presed key: %s", keyname);
            }
            else
            {
                SM_TRACE("presed keycode: %d", key);
            }
            
            break;
        }
        case GLFW_RELEASE:
        {
            
            const char * keyname = glfwGetKeyName(key, scancode);
            if (keyname)
            {
                SM_TRACE("released key: %s", keyname);
            }
            else
            {
                SM_TRACE("released keycode: %d", key);
            }
            
            break;
        }
        case GLFW_REPEAT:
        {
            
            const char * keyname = glfwGetKeyName(key, scancode);
            if (keyname)
            {
                SM_TRACE("repeated key: %s", keyname);
            }
            else
            {
                SM_TRACE("repeated keycode: %d", key);
            }
            
            break;
        }
        default: {}
    }
    
}

internal void InputMouseButtonCallback(GLFWwindow * _window, int32 button, int32 action, int32 mods)
{
    switch(action)
    {
        case GLFW_PRESS:
        {
            SM_TRACE("presed mouse button: %d", button);
            break;
        }
        case GLFW_RELEASE:
        {
            SM_TRACE("released mouse button: %d", button);
            break;
        }
        default: {}
    }
}

internal void InputScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    SM_TRACE("MouseScroll: (%.1f, %.1f)", xoffset, yoffset);
}

internal void InputMouseCursorCallback(GLFWwindow* window, double xpos, double ypos)
{
    // SM_TRACE("mouse position: (%.2f, %.2f)", xpos, ypos);
}

internal void InputSetJoystickCallback(int jid, int event)
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
    for ( ;!glfwWindowShouldClose(app.m_window); )
    {
        glfwPollEvents();
        //PrintAvailableJoyStics();
        ImguiStartFrame(app);

        // Rendering
        DrawFrame(app, &app.m_renderData);
    }
    vkDeviceWaitIdle(app.m_renderContext.m_device);
    }

void RunApplication(Application & app)
{
    // NOTE: Run Application
    InitWindow(app);
    InitRenderData(app);
    InitVulkan(app);
    InitImGui(app);
    MainLoop(app);
    CleanUp(app);
    }
