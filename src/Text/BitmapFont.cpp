//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-01-23.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "BitmapFont.hpp"

#include <filesystem>
#include <Tungsten/ArrayBufferBuilder.hpp>
#include <Yimage/ReadPng.hpp>
#include <Yson/JsonReader.hpp>
#include <Yson/ReaderIterators.hpp>
#include <Ystring/Ystring.hpp>

namespace
{
    std::pair<std::string, std::string>
    get_json_and_png_paths(std::filesystem::path path)
    {
        auto extension = ystring::to_lower(path.extension().string());
        if (extension == ".json")
        {
            const auto json_path = path;
            path.replace_extension(".png");
            return {json_path.string(), path.string()};
        }
        else if (extension == ".png")
        {
            const auto png_path = path;
            path.replace_extension(".json");
            return {path.string(), png_path.string()};
        }
        else
        {
            return {path.string() + ".json", path.string() + ".png"};
        }
    }
}

BitmapFont::BitmapFont(std::unordered_map<char32_t, CharData> char_data,
                       yimage::Image image)
    : char_data_(move(char_data)),
      image_(std::move(image))
{}

const CharData* BitmapFont::char_data(char32_t ch) const
{
    if (auto it = char_data_.find(ch); it != char_data_.end())
        return &it->second;
    return nullptr;
}

const std::unordered_map<char32_t, CharData>& BitmapFont::all_char_data() const
{
    return char_data_;
}

std::pair<int, int> BitmapFont::vertical_extremes() const
{
    {
        int max_hi = 0, min_lo = 0;
        for (auto& data: char_data_)
        {
            int hi = data.second.bearing_y;
            if (hi > max_hi)
                max_hi = hi;
            int lo = hi - int(data.second.height);
            if (lo < min_lo)
                min_lo = lo;
        }
        return {min_lo, max_hi};
    }
}

const yimage::Image& BitmapFont::image() const
{
    return image_;
}

yimage::Image BitmapFont::release_image()
{
    return std::move(image_);
}

std::unordered_map<char32_t, CharData> read_json_font(Yson::Reader& reader)
{
    using Yson::get;
    std::unordered_map<char32_t, CharData> result;
    for (const auto& key : keys(reader))
    {
        auto item = reader.readItem();
        auto position = item["position"];
        auto size = item["size"];
        auto bearing = item["bearing"];
        auto [range, ch] = ystring::get_code_point(key, 0);
        result.insert({ch,
                       {get<unsigned>(position[0].value()),
                        get<unsigned>(position[1].value()),
                        get<unsigned>(size[0].value()),
                        get<unsigned>(size[1].value()),
                        get<int>(bearing[0].value()),
                        get<int>(bearing[1].value()),
                        get<int>(item["advance"])}});
    }
    return result;
}

BitmapFont read_font(const std::string& font_path)
{
    auto[json_path, png_path] = get_json_and_png_paths(font_path);
    Yson::JsonReader reader(json_path);
    return BitmapFont(read_json_font(reader), yimage::read_png(png_path));
}
