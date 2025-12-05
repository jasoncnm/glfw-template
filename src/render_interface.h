/* date = November 30th 2025 7:44 pm */

#ifndef RENDER_INTERFACE_H

#include "engine_lib.h"
#include <glm/glm.hpp>

// constexpr char * TEXTURE_PATH = "resources/objects/backpack/diffuse_2.jpg";
// constexpr char * MODEL_PATH   = "resources/objects/backpack/backpack_2.obj";

// constexpr char * MODEL_PATH   = "resources/objects/cyborg/cyborg.obj";
 // constexpr char * TEXTURE_PATH = "resources/objects/cyborg/cyborg_diffuse.png";
  constexpr char * TEXTURE_PATH = "resources/objects/viking_room/viking_room.png";
 constexpr char * MODEL_PATH   = "resources/objects/viking_room/viking_room.obj";


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
    glm::vec3 m_up = { 0.0f, 0.0f, 1.0f };
    
     real32 m_fovy = 45.0f;
    real32 m_nearClip = 0.1f;
     real32 m_farClip = 100.0f;
    real32 m_pitch = 0.0f;
    real32 m_yaw = 0.0f;
    
};

struct Transform 
{
     std::vector<glm::vec3> m_meshPositions;
    Model m_model;
    uint32 m_numCopies;
    };

struct RenderData 
{
    glm::vec4 m_clearColor;
    Camera m_camera;
    
    // TODO: Current We can only Render one transform. 
    Transform m_transform;
    };

#define RENDER_INTERFACE_H
#endif //RENDER_INTERFACE_H
