cmake_minimum_required(VERSION 3.21)
project(Text)

set(CMAKE_CXX_STANDARD 20)

if (DEFINED LOCAL_SOURCE_PACKAGE_DIR OR DEFINED ENV{LOCAL_SOURCE_PACKAGE_DIR})
    if (NOT LOCAL_SOURCE_PACKAGE_DIR)
        set(LOCAL_SOURCE_PACKAGE_DIR $ENV{LOCAL_SOURCE_PACKAGE_DIR})
    endif ()
endif ()

include(FetchContent)

if (DEFINED LOCAL_SOURCE_PACKAGE_DIR)
    FetchContent_Declare(tungsten
        URL "file://${LOCAL_SOURCE_PACKAGE_DIR}/Tungsten-0.1.3.5.tar.gz")
    FetchContent_Declare(yimage
        URL "file://${LOCAL_SOURCE_PACKAGE_DIR}/Yimage-0.0.5.tar.gz")
else ()
    FetchContent_Declare(tungsten
        GIT_REPOSITORY "https://github.com/jebreimo/Tungsten.git"
        GIT_TAG v0.1.3)
    FetchContent_Declare(yimage
        GIT_REPOSITORY "https://github.com/jebreimo/Yimage.git"
        GIT_TAG main)
endif ()

FetchContent_Declare(argos
    GIT_REPOSITORY "https://github.com/jebreimo/Argos.git"
    GIT_TAG master)

FetchContent_MakeAvailable(argos tungsten yimage)

list(APPEND CMAKE_MODULE_PATH ${tungsten_SOURCE_DIR}/tools/cmake)

include(TungstenTargetEmbedShaders)

add_executable(Text
    src/Text/ShowTextShaderProgram.cpp
    src/Text/ShowTextShaderProgram.hpp
    src/Text/main.cpp
    )

target_link_libraries(Text
    PRIVATE
        Argos::Argos
        Tungsten::Tungsten
        Yimage::Yimage
    )

tungsten_target_embed_shaders(Text
    FILES
        src/Text/ShowText-frag.glsl
        src/Text/ShowText-vert.glsl
    )