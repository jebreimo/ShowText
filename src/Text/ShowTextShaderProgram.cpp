//****************************************************************************
// Copyright © 2021 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2021-10-09.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "ShowTextShaderProgram.hpp"
#include "ShowText-frag.glsl.hpp"
#include "ShowText-vert.glsl.hpp"

void ShowTextShaderProgram::setup()
{
    using namespace Tungsten;
    program = createProgram();
    auto vertexShader = createShader(GL_VERTEX_SHADER, ShowText_vert);
    attachShader(program, vertexShader);
    auto fragmentShader = createShader(GL_FRAGMENT_SHADER, ShowText_frag);
    attachShader(program, fragmentShader);
    linkProgram(program);
    useProgram(program);

    position = Tungsten::getVertexAttribute(program, "a_Position");
    textureCoord = Tungsten::getVertexAttribute(program, "a_TextureCoord");

    mvpMatrix = Tungsten::getUniform<Xyz::Matrix4f>(program, "u_MvpMatrix");
    texture = Tungsten::getUniform<GLint>(program, "u_Texture");
}
