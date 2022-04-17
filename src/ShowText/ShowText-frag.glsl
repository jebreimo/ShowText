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
uniform highp vec4 u_TextColor;

void main()
{
    highp vec4 texCol = texture2D(u_Texture, v_TextureCoord);
    highp float value = max(texCol.r, max(texCol.g, texCol.b));
    gl_FragColor = vec4(u_TextColor.r * value,
                        u_TextColor.g * value,
                        u_TextColor.b * value,
                        u_TextColor.a);
}
