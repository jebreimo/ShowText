//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-02-18.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <span>
#include <unordered_map>
#include <Tungsten/ArrayBuffer.hpp>
#include <Xyz/Rectangle.hpp>
#include <Xyz/Vector.hpp>
#include <Yimage/Image.hpp>
#include "BitmapFont.hpp"

struct GlCharData
{
    Xyz::Vector2F size;
    Xyz::Vector2F bearing;
    float advance = {};
    Xyz::Vector2F tex_origin;
    Xyz::Vector2F tex_size;
};

class GlFont
{
public:
    GlFont() = default;

    GlFont(std::unordered_map<char32_t, GlCharData> char_data,
           std::shared_ptr<BitmapFont> bitmap_font);

    [[nodiscard]]
    const GlCharData* char_data(char32_t ch) const;

    [[nodiscard]]
    const yimage::Image& image() const;

    [[nodiscard]]
    const std::shared_ptr<BitmapFont>& bitmap_font() const;
private:
    std::unordered_map<char32_t, GlCharData> char_data_;
    std::shared_ptr<BitmapFont> bitmap_font_;
};

GlFont make_gl_font(std::shared_ptr<BitmapFont> bitmap_font,
                    Xyz::Vector2F screen_size);

struct TextVertex
{
    Xyz::Vector2F pos;
    Xyz::Vector2F texture;
};

std::ostream& operator<<(std::ostream& os, const TextVertex& vertex);

std::ostream&
operator<<(std::ostream& os, const Tungsten::ArrayBuffer<TextVertex>& buffer);

Xyz::RectangleF get_text_size(const GlFont& font, std::u32string_view text);

Tungsten::ArrayBuffer<TextVertex>
format_text(const GlFont& font,
            std::u32string_view text,
            const Xyz::Vector2F& origin);
