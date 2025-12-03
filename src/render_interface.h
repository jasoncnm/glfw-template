/* date = November 30th 2025 7:44 pm */

#ifndef RENDER_INTERFACE_H

#include "engine_lib.h"
#include <glm/glm.hpp>

 // constexpr char * MODEL_PATH   = "resources/objects/cyborg/cyborg.obj";
 // constexpr char * TEXTURE_PATH = "resources/objects/cyborg/cyborg_diffuse.png";
 constexpr char * TEXTURE_PATH = "resources/objects/backpack/diffuse_2.jpg";
 constexpr char * MODEL_PATH   = "resources/objects/backpack/backpack.obj";

// constexpr char * TEXTURE_PATH = "resources/objects/viking_room/viking_room.png";
// constexpr char * MODEL_PATH   = "resources/objects/viking_room/viking_room.obj";

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
    
    float m_fovy = 45.0f;
    float m_nearClip = 0.1f;
    float m_farClip = 100.0f;
    
};

struct RenderData 
{
    glm::vec4 m_clearColor;
    Camera m_camera;
    // TODO: Need to make a transform struct to contains a model
    Model m_model;
    
    
};

#define RENDER_INTERFACE_H
#endif //RENDER_INTERFACE_H
