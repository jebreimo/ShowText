//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-01-23.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once

#include <span>
#include <unordered_map>
#include <Yimage/Image.hpp>
#include <Yson/Reader.hpp>
#include <Yson/Writer.hpp>

struct BitmapCharData
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

    BitmapFont(std::unordered_map<char32_t, BitmapCharData> char_data,
               Yimage::Image image);

    [[nodiscard]]
    const BitmapCharData* char_data(char32_t ch) const;

    [[nodiscard]]
    const std::unordered_map<char32_t, BitmapCharData>& all_char_data() const;

    [[nodiscard]]
    std::pair<int, int> vertical_extremes() const;

    [[nodiscard]]
    const Yimage::Image& image() const;

    Yimage::Image release_image();

private:
    std::unordered_map<char32_t, BitmapCharData> char_data_;
    Yimage::Image image_;
};

BitmapFont make_bitmap_font(const std::string& font_path,
                            unsigned font_size,
                            std::span<char32_t> chars);

BitmapFont read_bitmap_font(const std::string& font_path);

void write_font(const BitmapFont& font, const std::string& file_name);
