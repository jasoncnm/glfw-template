/* date = November 30th 2025 7:44 pm */

#ifndef RENDER_INTERFACE_H

#include "engine_lib.h"
#include <glm/glm.hpp>

 constexpr char * TEXTURE_PATH1 = "resources/objects/backpack/diffuse_2.jpg";
constexpr char * MODEL_PATH1   = "resources/objects/backpack/backpack_2.obj";

constexpr char * MODEL_PATH2   = "resources/objects/cyborg/cyborg.obj";
 constexpr char * TEXTURE_PATH2 = "resources/objects/cyborg/cyborg_diffuse.png";

constexpr char * TEXTURE_PATH3 = "resources/objects/viking_room/viking_room.png";
 constexpr char * MODEL_PATH3   = "resources/objects/viking_room/viking_room.obj";


#define MAX_TRANSFORM 1000


struct Vertex
{
    glm::vec3 m_pos;
    glm::vec3 m_color;
    glm::vec2 m_texCoord;
    
    bool operator==(const Vertex & other) const
    {
        return m_pos == other.m_pos && m_color == other.m_color && m_texCoord == other.m_texCoord;
    }
};

struct Model
{
    std::vector<Vertex> m_vertices;
    std::vector<uint32> m_indices;
};

struct Camera
{
    glm::vec3 m_pos = {};
    glm::vec3 m_forwardDirection = { 0.0f, 1.0f, 0.0f };
    
     real32 m_fov = 45.0f;
    real32 m_nearClip = 0.1f;
    real32 m_farClip = 100.0f;
    real32 m_pitch = 0.0f;
    real32 m_yaw = 0.0f;
    
};

struct Transform 
{
    char m_modelID[260];
    char m_textureID[260];
    uint32 m_numCopies;
    
    std::vector<glm::vec3> m_meshPositions;
    Model m_model;
    };

struct Fog
{
    real32 m_viewDistence = 50.0f;
    real32 m_steepness = 1.0f;
    glm::vec3 m_fogColor = glm::vec3(1.0f);
};

struct RenderData 
{
    real32 m_screenX = 0;
    real32 m_screenY = 0;
    real32 m_screenWidth = 0.0f;
    real32 m_screenHeight = 0.0f;
    glm::vec4 m_clearColor;
    Camera m_camera;
    Fog m_fog;
    
    // TODO: Current We can only Render one transform. 
    Array<Transform, MAX_TRANSFORM> m_transforms;
    };

#define RENDER_INTERFACE_H
#endif //RENDER_INTERFACE_H
