cmake_minimum_required(VERSION 3.20)

project(NPK LANGUAGES CXX C)

option(Build_NPK "Build renderer" ON)
option(Build_NPKPacker "Build packer tool" ON)
option(Build_NPKUnpacker "Build unpacker" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)

find_package(lz4 CONFIG REQUIRED)

if(Build_NPKPacker)

    add_executable(NPKPacker NPKPacker.cpp)

    target_include_directories(NPKPacker PUBLIC include)

    target_link_libraries(NPKPacker PUBLIC lz4::lz4)

endif()

if(Build_NPK)

    find_package(glfw3 CONFIG REQUIRED)
    find_package(OpenGL REQUIRED)

    add_library(glad STATIC src/external/glad.c)
    set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)

    add_library(imgui STATIC
        src/external/imgui.cpp
        src/external/imgui_demo.cpp
        src/external/imgui_draw.cpp
        src/external/imgui_impl_glfw.cpp
        src/external/imgui_impl_opengl3.cpp
        src/external/imgui_stdlib.cpp
        src/external/imgui_tables.cpp
        src/external/imgui_widgets.cpp
    )
    
    add_library(NPKLoader STATIC NPKLoader.cpp)

    target_include_directories(NPKLoader PUBLIC include)
    target_include_directories(imgui PUBLIC include)
    target_include_directories(glad PUBLIC include)

    target_link_libraries(imgui PUBLIC glfw)
    target_link_libraries(NPKLoader PUBLIC lz4::lz4)

    target_compile_definitions(NPKLoader PRIVATE NPK_BUILD)

    add_executable(NPK NPK.cpp)

    target_include_directories(NPK PUBLIC include)

    target_link_libraries(NPK PUBLIC
        OpenGL::GL
        glfw
        glad
        imgui
        NPKLoader
    )

endif()

if(Build_NPKUnpacker)
    add_executable(NPKUnpacker NPKLoader.cpp)

    target_include_directories(NPKUnpacker PUBLIC include)

    target_link_libraries(NPKUnpacker PUBLIC lz4::lz4)

endif()
