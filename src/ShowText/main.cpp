//****************************************************************************
// Copyright © 2021 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2021-12-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <filesystem>
#include <iostream>
#include <Argos/Argos.hpp>
#include <Tungsten/SdlApplication.hpp>
#include <Yimage/Yimage.hpp>
#include <Ystring/Ystring.hpp>
#include "BitmapFont.hpp"
#include "ShowTextShaderProgram.hpp"
#include "GlFont.hpp"

namespace
{
    std::pair<int, int> get_ogl_pixel_type(yimage::PixelType type)
    {
        switch (type)
        {
        case yimage::PixelType::MONO_8:
            return {GL_RED, GL_UNSIGNED_BYTE};
        case yimage::PixelType::MONO_ALPHA_8:
            return {GL_RG, GL_UNSIGNED_BYTE};
        case yimage::PixelType::RGB_8:
            return {GL_RGB, GL_UNSIGNED_BYTE};
        case yimage::PixelType::RGBA_8:
            return {GL_RGBA, GL_UNSIGNED_BYTE};
        case yimage::PixelType::MONO_1:
        case yimage::PixelType::MONO_2:
        case yimage::PixelType::MONO_4:
        case yimage::PixelType::MONO_16:
        case yimage::PixelType::ALPHA_MONO_8:
        case yimage::PixelType::ALPHA_MONO_16:
        case yimage::PixelType::MONO_ALPHA_16:
        case yimage::PixelType::RGB_16:
        case yimage::PixelType::ARGB_8:
        case yimage::PixelType::ARGB_16:
        case yimage::PixelType::RGBA_16:
        default:
            break;
        }
        throw std::runtime_error("GLES has no corresponding pixel format: "
                                 + std::to_string(int(type)));
    }
}

class ShowText : public Tungsten::EventLoop
{
public:
    ShowText(BitmapFont font, std::string text)
        : bmp_font_(std::move(font)),
          text_(move(text))
    {}

    void on_startup(Tungsten::SdlApplication& app) override
    {
        int w, h;
        SDL_GetWindowSize(app.window(), &w, &h);

        font_ = make_gl_font(std::move(bmp_font_), {float(w), float(h)});
        auto text_size = get_text_size(font_, text_);
        auto origin = Xyz::make_vector2(-text_size.size()[0] / 2.f,
                                        -text_size.size()[1] / 2.f - text_size.min()[1]);
        auto buffer = format_text(font_, text_, origin);
        std::cout << buffer << '\n';
        count_ = int32_t(buffer.indexes.size());
        vertex_array_ = Tungsten::generate_vertex_array();
        Tungsten::bind_vertex_array(vertex_array_);
        buffers_ = Tungsten::generate_buffers(2);
        set_buffers(buffers_[0], buffers_[1], buffer);
        program_.setup();
        Tungsten::set_texture_min_filter(GL_TEXTURE_2D, GL_LINEAR);
        Tungsten::set_texture_mag_filter(GL_TEXTURE_2D, GL_LINEAR);
        Tungsten::set_texture_parameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        Tungsten::set_texture_parameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        const auto& image = font_.image();
        auto [format, type] = get_ogl_pixel_type(image.pixel_type());
        Tungsten::set_texture_image_2d(GL_TEXTURE_2D, 0, GL_RED,
                                       GLsizei(image.width()),
                                       GLsizei(image.height()),
                                       format, type,
                                       image.data());

        program_.setup();
        Tungsten::define_vertex_attribute_pointer(
            program_.position, 2, GL_FLOAT, false, 4 * sizeof(float), 0);
        Tungsten::enable_vertex_attribute(program_.position);
        Tungsten::define_vertex_attribute_pointer(
            program_.texture_coord, 2, GL_FLOAT, false, 4 * sizeof(float),
            2 * sizeof(float));
        Tungsten::enable_vertex_attribute(program_.texture_coord);

        auto m = float(std::min(w, h));
        auto projection = Xyz::make_frustum_matrix<float>(-1, 1, -1, 1, 1, 5)
                          * Xyz::make_look_at_matrix(Xyz::make_vector3<float>(0, 0, 1),
                                                  Xyz::make_vector3<float>(0, 0, 0),
                                                  Xyz::make_vector3<float>(0, 1, 0))
                          * Xyz::scale4<float>(float(h) / m, float(w) / m, 1.f);

        program_.mvp_matrix.set(projection);
    }

    void on_draw(Tungsten::SdlApplication& app) override
    {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Tungsten::draw_elements(GL_TRIANGLES, count_, GL_UNSIGNED_SHORT);
    }
private:
    BitmapFont bmp_font_;
    GlFont font_;
    std::string text_;
    std::vector<Tungsten::BufferHandle> buffers_;
    Tungsten::VertexArrayHandle vertex_array_;
    ShowTextShaderProgram program_;
    int32_t count_ = 0;
};

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

int main(int argc, char* argv[])
{
    try
    {
        argos::ArgumentParser parser(argv[0]);
        parser.about("Creates an OpenGL window where it displays a"
                     " given text with a given bitmap font.")
            .add(argos::Argument("FONT")
                .help("Path to a bitmap font. This can be either the"
                      " PNG file, the JSON file, or just the font name"
                      " without the extension."))
            .add(argos::Argument("TEXT")
                .count(1, UINT_MAX)
                .help("The text the program will display."));
        Tungsten::SdlApplication::add_command_line_options(parser);
        auto args = parser.parse(argc, argv);

        auto bmp_font = read_font(args.value("FONT").as_string());

        auto [lo, hi] = bmp_font.vertical_extremes();
        std::cout << lo << ", " << hi << "\n";

        auto event_loop = std::make_unique<ShowText>(
            std::move(bmp_font),
            args.value("TEXT").as_string());
        Tungsten::SdlApplication app("ShowPng", std::move(event_loop));
        app.read_command_line_options(args);
        app.run();
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << "\n";
        return 1;
    }
    return 0;
}
