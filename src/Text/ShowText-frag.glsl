//****************************************************************************
// Copyright © 2021 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2021-10-09.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 100

varying highp vec2 v_TextureCoord;

uniform sampler2D u_Texture;

void main()
{
    gl_FragColor = texture2D(u_Texture, v_TextureCoord);
}
