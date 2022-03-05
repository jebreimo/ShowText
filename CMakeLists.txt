cmake_minimum_required(VERSION 3.21)
project(ShowText)

set(CMAKE_CXX_STANDARD 20)

if (DEFINED LOCAL_SOURCE_PACKAGE_DIR OR DEFINED ENV{LOCAL_SOURCE_PACKAGE_DIR})
    if (NOT LOCAL_SOURCE_PACKAGE_DIR)
        set(LOCAL_SOURCE_PACKAGE_DIR $ENV{LOCAL_SOURCE_PACKAGE_DIR})
    endif ()
endif ()

include(FetchContent)

if (DEFINED LOCAL_SOURCE_PACKAGE_DIR)
    FetchContent_Declare(tungsten
        URL "file://${LOCAL_SOURCE_PACKAGE_DIR}/Tungsten-0.2.4.tar.gz")
    FetchContent_Declare(yimage
        URL "file://${LOCAL_SOURCE_PACKAGE_DIR}/Yimage-0.0.6.tar.gz")
else ()
    FetchContent_Declare(tungsten
        GIT_REPOSITORY "https://github.com/jebreimo/Tungsten.git"
        GIT_TAG v0.2.4)
    FetchContent_Declare(yimage
        GIT_REPOSITORY "https://github.com/jebreimo/Yimage.git"
        GIT_TAG main)
endif ()

FetchContent_Declare(argos
    GIT_REPOSITORY "https://github.com/jebreimo/Argos.git"
    GIT_TAG master)
FetchContent_Declare(yson
    GIT_REPOSITORY "https://github.com/jebreimo/Yson.git"
    GIT_TAG master)
FetchContent_Declare(ystring2
    GIT_REPOSITORY "https://github.com/jebreimo/Ystring2.git"
    GIT_TAG master)

FetchContent_MakeAvailable(argos tungsten yimage yson ystring2)

list(APPEND CMAKE_MODULE_PATH ${tungsten_SOURCE_DIR}/tools/cmake)

include(TungstenTargetEmbedShaders)

add_executable(ShowText
    src/ShowText/ShowTextShaderProgram.cpp
    src/ShowText/ShowTextShaderProgram.hpp
    src/ShowText/main.cpp
    src/ShowText/BitmapFont.cpp
    src/ShowText/BitmapFont.hpp
    src/ShowText/GlFont.cpp
    src/ShowText/GlFont.hpp
    )

target_link_libraries(ShowText
    PRIVATE
        Argos::Argos
        Tungsten::Tungsten
        Yimage::Yimage
        Yson::Yson
        Ystring2::Ystring
    )

tungsten_target_embed_shaders(ShowText
    FILES
        src/ShowText/ShowText-frag.glsl
        src/ShowText/ShowText-vert.glsl
    )
