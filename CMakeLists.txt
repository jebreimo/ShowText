cmake_minimum_required(VERSION 3.21)
project(ShowText VERSION 0.0.2)

set(CMAKE_CXX_STANDARD 20)

find_package(Freetype REQUIRED)

include(FetchContent)

FetchContent_Declare(argos
    GIT_REPOSITORY "https://github.com/jebreimo/Argos.git"
    GIT_TAG master)
FetchContent_Declare(tungsten
    GIT_REPOSITORY "https://github.com/jebreimo/Tungsten.git"
    GIT_TAG v0.3.1)
FetchContent_Declare(yimage
    GIT_REPOSITORY "https://github.com/jebreimo/Yimage.git"
    GIT_TAG main)
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
    src/ShowText/BitmapFont.cpp
    src/ShowText/BitmapFont.hpp
    src/ShowText/FreeTypeWrapper.cpp
    src/ShowText/FreeTypeWrapper.hpp
    src/ShowText/GlFont.cpp
    src/ShowText/GlFont.hpp
    src/ShowText/main.cpp
    src/ShowText/ShowTextShaderProgram.cpp
    src/ShowText/ShowTextShaderProgram.hpp
    )

target_link_libraries(ShowText
    PRIVATE
        Freetype::Freetype
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
