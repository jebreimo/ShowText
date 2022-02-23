//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-01-23.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <unordered_map>
#include <Xyz/Vector.hpp>
#include <Tungsten/ArrayBuffer.hpp>
#include <string_view>
#include <Yimage/Image.hpp>

struct CharData
{
    unsigned x = 0;
    unsigned y = 0;
    unsigned width = 0;
    unsigned height = 0;
    int bearing_x = 0;
    int bearing_y = 0;
    int advance = 0;
};

class BitmapFont
{
public:
    BitmapFont() = default;

    explicit BitmapFont(std::unordered_map<char32_t, CharData> char_data,
                        yimage::Image image);

    [[nodiscard]]
    const CharData* char_data(char32_t ch) const;

    [[nodiscard]]
    const std::unordered_map<char32_t, CharData>& all_char_data() const;

    [[nodiscard]]
    std::pair<int, int> vertical_extremes() const;

    [[nodiscard]]
    const yimage::Image& image() const;

    yimage::Image release_image();
private:
    std::unordered_map<char32_t, CharData> char_data_;
    yimage::Image image_;
};

BitmapFont read_font(const std::string& font_path);
//
//Tungsten::ArrayBuffer<TextVertex>
//format_text(const BitmapFont& font,
//            std::string_view text,
//            const Xyz::Vector2f& origin);
