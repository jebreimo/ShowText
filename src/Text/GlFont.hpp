//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-02-18.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <unordered_map>
#include <Tungsten/ArrayBuffer.hpp>
#include <Xyz/Vector.hpp>
#include <Yimage/Image.hpp>
#include "BitmapFont.hpp"

struct GlCharData
{
    Xyz::Vector2f size;
    Xyz::Vector2f bearing;
    float advance = {};
    Xyz::Vector2f tex_origin;
    Xyz::Vector2f tex_size;
};

class GlFont
{
public:
    GlFont() = default;

    GlFont(std::unordered_map<char32_t, GlCharData> char_data,
           yimage::Image image);

    [[nodiscard]]
    const GlCharData* char_data(char32_t ch) const;

    [[nodiscard]]
    const yimage::Image& image() const;
private:
    std::unordered_map<char32_t, GlCharData> char_data_;
    yimage::Image image_;
};

GlFont make_gl_font(BitmapFont bitmap_font, Xyz::Vector2f screen_size);

struct TextVertex
{
    Xyz::Vector2f pos;
    Xyz::Vector2f texture;
};

std::ostream& operator<<(std::ostream& os, const TextVertex& vertex);

std::ostream&
operator<<(std::ostream& os, const Tungsten::ArrayBuffer<TextVertex>& buffer);

Tungsten::ArrayBuffer<TextVertex>
format_text(const GlFont& font,
            std::string_view text,
            const Xyz::Vector2f& origin);
