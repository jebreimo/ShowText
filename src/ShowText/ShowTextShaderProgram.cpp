//****************************************************************************
// Copyright © 2021 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2021-10-09.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "ShowTextShaderProgram.hpp"

#include <Tungsten/ShaderProgramBuilder.hpp>
#include "ShowText-frag.glsl.hpp"
#include "ShowText-vert.glsl.hpp"

void ShowTextShaderProgram::setup()
{
    using namespace Tungsten;
    program = ShaderProgramBuilder()
        .add_shader(ShaderType::VERTEX, ShowText_vert)
        .add_shader(ShaderType::FRAGMENT, ShowText_frag)
        .build();

    use_program(program);

    position = Tungsten::get_vertex_attribute(program, "a_Position");
    texture_coord = Tungsten::get_vertex_attribute(program, "a_TextureCoord");

    mvp_matrix = Tungsten::get_uniform<Xyz::Matrix4F>(program, "u_MvpMatrix");
    texture = Tungsten::get_uniform<GLint>(program, "u_Texture");
    color = Tungsten::get_uniform<Xyz::Vector4F>(program, "u_TextColor");
}
