cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED)
project(glfw_template CXX)

# Download glfw
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/jasoncnm/glfw.git
)
FetchContent_MakeAvailable(glfw)

# Download glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/jasoncnm/glm.git
)
FetchContent_MakeAvailable(glm)

find_package (Vulkan REQUIRED)

# Setup Vulkan include and link directory
include_directories($ENV{VULKAN_SDK}/Include)
link_directories($ENV{VULKAN_SDK}/Lib)

# Setup additional include directories

include_directories(third_party/imgui)
include_directories(third_party/stb)

#include and link  
add_executable(${PROJECT_NAME} src/main.cpp src/build_imgui_backends.cpp)

target_link_libraries(${PROJECT_NAME} glfw glm)
target_link_libraries(${PROJECT_NAME} vulkan-1.lib)


if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -DAPP_SLOW=1")
endif()

if ( MSVC )
    set(CMAKE_EXE_LINKER_FLAGS  "-incremental:no -opt:ref")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}  -fsanitize=address -MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4324 -wd4996 -wd4456 -wd4505 -wd4201 -wd4100 -wd4189 -DAPP_SLOW=1 -FC -Z7")
endif()

set_target_properties(${PROJECT_NAME} 
                      PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${PROJECT_NAME} 
                      PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
