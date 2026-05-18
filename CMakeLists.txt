cmake_minimum_required(VERSION 3.20)
project(Rvelent VERSION 0.0.1 LANGUAGES CXX C)

# --- 1. SETTINGS ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)

# Wicked Engine Settings (Must be set BEFORE add_subdirectory)
set(WICKED_EDITOR OFF CACHE BOOL "" FORCE) 
set(WICKED_IMGUI_EXAMPLE OFF CACHE BOOL "" FORCE)
set(WICKED_TESTS OFF CACHE BOOL "" FORCE)
set(WICKED_WINDOWS_TEMPLATE OFF CACHE BOOL "" FORCE)
set(WICKED_LINUX_TEMPLATE OFF CACHE BOOL "" FORCE)

# --- 2. DEPENDENCIES ---
include(FetchContent)

# SDL3 (Windowing, Input, Audio)
FetchContent_Declare(SDL3 GIT_REPOSITORY https://github.com/libsdl-org/SDL.git GIT_TAG release-3.2.0)
FetchContent_MakeAvailable(SDL3)

# Dear ImGui (Editor UI)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG docking)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

# Wicked Engine
FetchContent_Declare(
    WickedEngine
    GIT_REPOSITORY https://github.com/Turanszkij/WickedEngine.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(WickedEngine)

# --- 3. SOURCE FILES ---
file(GLOB_RECURSE ENGINE_SOURCES 
    "${CMAKE_SOURCE_DIR}/Source/Core/*.cpp" 
    "${CMAKE_SOURCE_DIR}/Source/Core/*.h"
    "${CMAKE_SOURCE_DIR}/Source/Runtime/*.cpp" 
    "${CMAKE_SOURCE_DIR}/Source/Runtime/*.h"
    "${CMAKE_SOURCE_DIR}/Source/Modules/*.cpp" 
    "${CMAKE_SOURCE_DIR}/Source/Modules/*.h"
    "${CMAKE_SOURCE_DIR}/Source/Editor/*.cpp" 
    "${CMAKE_SOURCE_DIR}/Source/Editor/*.h"
    # Exclude old main/launch if they conflict (we will rewrite Launch.cpp next)
)

# --- 4. CREATE EXECUTABLE ---
# Include ImGui sources in the build
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
)

add_executable(RvelentEditor ${ENGINE_SOURCES} ${IMGUI_SOURCES})

# --- 5. INCLUDE PATHS ---
target_include_directories(RvelentEditor PRIVATE 
    ${CMAKE_SOURCE_DIR}/Source
    ${imgui_SOURCE_DIR}                  # For imgui.h
    ${imgui_SOURCE_DIR}/backends         # For imgui_impl_sdl3.h
    ${CMAKE_SOURCE_DIR}/Source/ThirdParty/WickedEngine
    ${CMAKE_SOURCE_DIR}/Source/ThirdParty/WickedEngine/WickedEngine
)

# --- 6. LINKING ---
# Link SDL3 and Wicked Engine
if(WIN32)
    target_link_libraries(RvelentEditor PRIVATE SDL3::SDL3 WickedEngine_Windows)
else()
    target_link_libraries(RvelentEditor PRIVATE SDL3::SDL3 WickedEngine_Linux) 
endif()

# Find DXC (DirectX Shader Compiler)
# Try Windows SDK first, then standalone DXC
find_program(DXC_EXECUTABLE
    NAMES dxc dxc.exe
    PATHS
        "C:/Program Files (x86)/Windows Kits/10/bin/10.0.22621.0/x64"
        "C:/Program Files (x86)/Windows Kits/10/bin/10.0.22000.0/x64"
        "C:/Program Files (x86)/Windows Kits/10/bin/x64"
        "$ENV{WindowsSdkVerBinPath}/x64"
        "$ENV{WindowsSdkBinPath}/x64"
    DOC "DirectX Shader Compiler (DXC)"
)

if(DXC_EXECUTABLE)
    message(STATUS "Found DXC: ${DXC_EXECUTABLE}")
    
    # Function to compile a shader
    function(compile_shader SHADER_SOURCE SHADER_OUTPUT SHADER_PROFILE)
        add_custom_command(
            OUTPUT ${SHADER_OUTPUT}
            COMMAND ${DXC_EXECUTABLE} 
                -T ${SHADER_PROFILE}
                -E main
                -Fo ${SHADER_OUTPUT}
                ${SHADER_SOURCE}
            DEPENDS ${SHADER_SOURCE}
            COMMENT "Compiling shader: ${SHADER_SOURCE} -> ${SHADER_OUTPUT}"
            VERBATIM
        )
    endfunction()
    
    # Compile ImGui shaders to Bin/Debug (the working directory)
    set(IMGUI_VS_SOURCE "${CMAKE_SOURCE_DIR}/Source/Modules/Render_Wicked/ImGuiVS.hlsl")
    set(IMGUI_VS_OUTPUT "${CMAKE_BINARY_DIR}/../Bin/Debug/ImGuiVS.cso")
    set(IMGUI_PS_SOURCE "${CMAKE_SOURCE_DIR}/Source/Modules/Render_Wicked/ImGuiPS.hlsl")
    set(IMGUI_PS_OUTPUT "${CMAKE_BINARY_DIR}/../Bin/Debug/ImGuiPS.cso")
    
    compile_shader(${IMGUI_VS_SOURCE} ${IMGUI_VS_OUTPUT} "vs_6_0")
    compile_shader(${IMGUI_PS_SOURCE} ${IMGUI_PS_OUTPUT} "ps_6_0")
    
    # Create a custom target that depends on the compiled shaders
    add_custom_target(CompileImGuiShaders ALL
        DEPENDS ${IMGUI_VS_OUTPUT} ${IMGUI_PS_OUTPUT}
    )
    
    # Make RvelentEditor depend on the shaders
    add_dependencies(RvelentEditor CompileImGuiShaders)
    
    message(STATUS "ImGui shaders will be compiled at build time")
else()
    message(WARNING "DXC not found! Shaders will not be compiled. Install Windows SDK or download DXC from https://github.com/microsoft/DirectXShaderCompiler/releases")
endif()
