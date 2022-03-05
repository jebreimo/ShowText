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
    program = create_program();
    auto vertex_shader = create_shader(GL_VERTEX_SHADER, ShowText_vert);
    attach_shader(program, vertex_shader);
    auto fragment_shader = create_shader(GL_FRAGMENT_SHADER, ShowText_frag);
    attach_shader(program, fragment_shader);
    link_program(program);
    use_program(program);

    position = Tungsten::get_vertex_attribute(program, "a_Position");
    texture_coord = Tungsten::get_vertex_attribute(program, "a_TextureCoord");

    mvp_matrix = Tungsten::get_uniform<Xyz::Matrix4F>(program, "u_MvpMatrix");
    texture = Tungsten::get_uniform<GLint>(program, "u_Texture");
}
