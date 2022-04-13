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
#include "FreeTypeWrapper.hpp"

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

GlCharData make_char_data(FT_GlyphSlot glyph, unsigned x, unsigned y,
                          Xyz::Vector2F screen_size,
                          Xyz::Vector2F image_size)
{
    auto bmp = glyph->bitmap;
    return {.size = {float(2 * bmp.width) / screen_size[0],
                     float(2 * bmp.rows) / screen_size[1]},
        .bearing = {float(2 * glyph->bitmap_left) / screen_size[0],
                    float(2 * glyph->bitmap_top) / screen_size[1]},
        .advance = float(2 * glyph->advance.x) / (64.0f * screen_size[0]),
        .tex_origin = {float(x) / image_size[0],
                       float(y + bmp.rows) / image_size[1]},
        .tex_size = {float(bmp.width) / image_size[0],
                     -float(bmp.rows) / image_size[1]}};
}

struct CharData
{
    Xyz::Vector<unsigned, 2> size;
    Xyz::Vector<int, 2> bearing;
    int advance = 0;
};

CharData get_size(freetype::Face& face, char32_t c)
{
    face.load_char(c, FT_LOAD_BITMAP_METRICS_ONLY);
    auto glyph = face->glyph;
    auto bmp = glyph->bitmap;
    return {.size = {bmp.width, bmp.rows},
        .bearing = {glyph->bitmap_left, glyph->bitmap_top},
        .advance = int(glyph->advance.x)};
}

std::pair<unsigned, unsigned>
get_max_glyph_size(freetype::Face& face, std::span<char32_t> chars)
{
    unsigned width = 0, height = 0;
    for (auto ch : chars)
    {
        auto cdata = get_size(face, ch);
        if (cdata.size[0] > width)
            width = cdata.size[0];
        if (cdata.size[1] > height)
            height = cdata.size[1];
    }
    return {width, height};
}

std::pair<unsigned, unsigned> get_best_grid_size(unsigned count)
{
    if (count == 0)
        return {0, 0};

    struct Best
    {
        unsigned width;
        unsigned height;
        unsigned remainder;
    };
    Best best = {count, 1, count};
    auto width = unsigned(ceil(sqrt(count)));
    while (true)
    {
        auto height = count / width;
        const auto n = count % width;
        if (n)
            ++height;
        if (width > height * 2)
            break;
        if (n == 0)
            return {width, height};
        if (width - n < best.remainder)
            best = {width, height, width - n};
        ++width;
    }
    return {best.width, best.height};
}

GlFont make_gl_font(const std::string& font_path,
                    unsigned int font_size,
                    Xyz::Vector2F screen_size,
                    std::span<char32_t> chars)
{
    freetype::Library library;
    auto face = library.new_face(font_path, 0);
    face.select_charmap(FT_ENCODING_UNICODE);
    face.set_pixel_sizes(0, font_size);

    auto [glyph_width, glyph_height] = get_max_glyph_size(face, chars);
    glyph_width += 1;
    glyph_height += 1;

    const auto [grid_width, grid_height] = get_best_grid_size(chars.size());
    auto image_width = glyph_width * grid_width;
    if (auto n = image_width % 8)
        image_width += 8 - n;
    yimage::Image image(yimage::PixelType::MONO_8,
                        image_width,
                        grid_height * glyph_height);
    yimage::MutableImageView mut_image = image;
    Xyz::Vector2F image_size = {float(image.width()), float(image.height())};

    std::unordered_map<char32_t, GlCharData> char_map;
    for (unsigned i = 0; i < chars.size(); ++i)
    {
        const auto x = (i % grid_width) * glyph_width;
        const auto y = (i / grid_width) * glyph_height;

        const auto ch = chars[i];
        face.load_char(ch, FT_LOAD_RENDER);
        auto glyph = face->glyph;
        const auto ch_data = make_char_data(glyph, x, y, screen_size,
                                            image_size);
        char_map.insert({ch, ch_data});
        yimage::ImageView glyph_img(glyph->bitmap.buffer,
                                    yimage::PixelType::MONO_8,
                                    glyph->bitmap.width,
                                    glyph->bitmap.rows);
        paste(glyph_img, x, y, mut_image);
    }

    return {std::move(char_map), std::move(image)};
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
                 std::u32string_view text,
                 Xyz::Vector2F origin)
{
    for (const auto c : text)
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

Xyz::RectangleF get_text_size(const GlFont& font, std::u32string_view text)
{
    Xyz::Vector2F min, max;

    for (const auto c : text)
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
            std::u32string_view text,
            const Xyz::Vector2F& origin)
{
    Tungsten::ArrayBuffer<TextVertex> result;
    format_text(result, font, text, origin);
    return result;
}
