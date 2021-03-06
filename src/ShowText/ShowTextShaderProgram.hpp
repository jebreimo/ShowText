//****************************************************************************
// Copyright © 2021 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2021-10-09.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Tungsten/Tungsten.hpp"

class ShowTextShaderProgram
{
public:
    void setup();

    Tungsten::ProgramHandle program;

    Tungsten::Uniform<Xyz::Matrix4F> mvp_matrix;
    Tungsten::Uniform<GLint> texture;
    Tungsten::Uniform<Xyz::Vector4F> color;

    GLuint position;
    GLuint texture_coord;
};
