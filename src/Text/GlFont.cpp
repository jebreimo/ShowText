//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-02-18.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "GlFont.hpp"

#include <Tungsten/ArrayBufferBuilder.hpp>
#include <Ystring/Ystring.hpp>

GlFont::GlFont(std::unordered_map<char32_t, GlCharData> char_data,
               yimage::Image image)
    : char_data_(move(char_data)),
      image_(std::move(image))
{}

const GlCharData* GlFont::char_data(char32_t ch) const
{
    if (auto it = char_data_.find(ch); it != char_data_.end())
        return &it->second;
    return nullptr;
}

const yimage::Image& GlFont::image() const
{
    return image_;
}

GlFont make_gl_font(BitmapFont bitmap_font, Xyz::Vector2F screen_size)
{
    std::unordered_map<char32_t, GlCharData> char_data;
    const auto& img = bitmap_font.image();
    if (!img.size())
        throw std::runtime_error("BitmapFont instance doesn't contain an image.");

    for (const auto [ch, data] : bitmap_font.all_char_data())
    {
        GlCharData gd;
        gd.tex_origin = {float(data.x) / float(img.width()),
                         float(data.y + data.height) / float(img.height())};
        gd.tex_size = {float(data.width) / float(img.width()),
                       -float(data.height) / float(img.height())};
        gd.advance = float(2 * float(data.advance) / (64 * screen_size[0]));
        gd.size = {2 * float(data.width) / screen_size[0],
                   2 * float(data.height) / screen_size[1]};
        gd.bearing = {2 * float(data.bearing_x) / screen_size[0],
                      2 * float(data.bearing_y) / screen_size[1]};
        char_data.insert({ch, gd});
    }
    return {move(char_data), bitmap_font.release_image()};
}

std::ostream& operator<<(std::ostream& os, const TextVertex& vertex)
{
    return os << vertex.pos << " -- " << vertex.texture;
}

std::ostream&
operator<<(std::ostream& os,
           const Tungsten::ArrayBuffer<TextVertex>& buffer)
{
    for (const auto& v: buffer.vertexes)
        os << v << '\n';
    for (size_t i = 0, m = buffer.indexes.size() / 3; i < m; ++i)
    {
        size_t j = i * 3;
        os << buffer.indexes[j] << ", " << buffer.indexes[j + 1]
           << ", " << buffer.indexes[j + 2] << '\n';
    }
    return os;
}

void add_rectangle(Tungsten::ArrayBufferBuilder<TextVertex> builder,
                   const Xyz::Vector2F& origin,
                   const Xyz::Vector2F& size,
                   const Xyz::Vector2F& tex_origin,
                   const Xyz::Vector2F& tex_size)
{
    using V = Xyz::Vector2F;

    builder.reserve_vertexes(4);
    builder.add_vertex({origin, tex_origin});
    builder.add_vertex({origin + V{size[0], 0},
                        tex_origin + V{tex_size[0], 0}});
    builder.add_vertex({origin + V{0, size[1]},
                        tex_origin + V{0, tex_size[1]}});
    builder.add_vertex({origin + size, tex_origin + tex_size});
    builder.reserve_indexes(6);
    builder.add_indexes(0, 1, 2);
    builder.add_indexes(2, 1, 3);
}

void format_text(Tungsten::ArrayBuffer<TextVertex>& buffer,
                 const GlFont& font,
                 std::string_view text,
                 Xyz::Vector2F origin)
{
    for (const auto c: ystring::to_utf32(text))
    {
        auto cdata = font.char_data(c);
        if (!cdata)
            continue;
        add_rectangle(Tungsten::ArrayBufferBuilder<TextVertex>{buffer},
                      {origin[0] + cdata->bearing[0],
                       origin[1] + cdata->bearing[1] - cdata->size[1]},
                      cdata->size,
                      cdata->tex_origin,
                      cdata->tex_size);
        origin[0] += cdata->advance;
    }
}

Xyz::RectangleF get_text_size(const GlFont& font, std::string_view text)
{
    Xyz::Vector2F min, max;

    for (const auto c: ystring::to_utf32(text))
    {
        auto cdata = font.char_data(c);
        if (!cdata)
            continue;
        max[0] += cdata->advance;
        auto hi = cdata->bearing[1];
        if (hi > max[1])
            max[1] = hi;
        auto lo = cdata->size[1] - cdata->bearing[1];
        if (lo < min[1])
            min[1] = lo;
    }
    return {min, max - min};
}

Tungsten::ArrayBuffer<TextVertex>
format_text(const GlFont& font,
            std::string_view text,
            const Xyz::Vector2F& origin)
{
    Tungsten::ArrayBuffer<TextVertex> result;
    format_text(result, font, text, origin);
    return result;
}
