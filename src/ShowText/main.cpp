//****************************************************************************
// Copyright © 2021 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2021-12-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <iostream>
#include <unordered_set>
#include <Argos/Argos.hpp>
#include <Tungsten/SdlApplication.hpp>
#include <Yimage/Yimage.hpp>
#include <Ystring/Ystring.hpp>
#include "BitmapFont.hpp"
#include "ShowTextShaderProgram.hpp"
#include "GlFont.hpp"

namespace
{
    std::pair<int, int> get_ogl_pixel_type(Yimage::PixelType type)
    {
        switch (type)
        {
        case Yimage::PixelType::MONO_8:
            return {GL_RED, GL_UNSIGNED_BYTE};
        case Yimage::PixelType::MONO_ALPHA_8:
            return {GL_RG, GL_UNSIGNED_BYTE};
        case Yimage::PixelType::RGB_8:
            return {GL_RGB, GL_UNSIGNED_BYTE};
        case Yimage::PixelType::RGBA_8:
            return {GL_RGBA, GL_UNSIGNED_BYTE};
        case Yimage::PixelType::MONO_1:
        case Yimage::PixelType::MONO_2:
        case Yimage::PixelType::MONO_4:
        case Yimage::PixelType::MONO_16:
        case Yimage::PixelType::ALPHA_MONO_8:
        case Yimage::PixelType::ALPHA_MONO_16:
        case Yimage::PixelType::MONO_ALPHA_16:
        case Yimage::PixelType::RGB_16:
        case Yimage::PixelType::ARGB_8:
        case Yimage::PixelType::ARGB_16:
        case Yimage::PixelType::RGBA_16:
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
    ShowText(std::shared_ptr<BitmapFont> font, std::u32string text)
        : bmp_font_(std::move(font)),
          text_(std::move(text))
    {}

    void on_startup(Tungsten::SdlApplication& app) override
    {
        int w, h;
        SDL_GetWindowSize(app.window(), &w, &h);

        font_ = make_gl_font(bmp_font_, {float(w), float(h)});
        auto text_size = get_text_size(font_, text_);
        auto origin = Xyz::make_vector2(-text_size.size()[0] / 2.f,
                                        -text_size.size()[1] / 2.f - text_size.min()[1]);
        auto buffer = format_text(font_, text_, origin);
        count_ = int32_t(buffer.indexes.size());
        vertex_array_ = Tungsten::generate_vertex_array();
        Tungsten::bind_vertex_array(vertex_array_);
        buffers_ = Tungsten::generate_buffers(2);
        set_buffers(buffers_[0], buffers_[1], buffer);

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
        program_.color.set({1.0, 1.0, 1.0, 1.0});
    }

    bool on_event(Tungsten::SdlApplication& app, const SDL_Event& event) override
    {
        if (event.type == SDL_WINDOWEVENT
            && event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
            glViewport(0, 0, event.window.data1, event.window.data2);

            auto [w, h] = app.window_size();
            font_ = make_gl_font(std::move(bmp_font_), {float(w), float(h)});
            auto text_size = get_text_size(font_, text_);
            auto origin = Xyz::make_vector2(-text_size.size()[0] / 2.f,
                                            -text_size.size()[1] / 2.f - text_size.min()[1]);
            auto buffer = format_text(font_, text_, origin);
            count_ = int32_t(buffer.indexes.size());
            set_buffers(buffers_[0], buffers_[1], buffer);
        }

        return EventLoop::on_event(app, event);
    }

    void on_draw(Tungsten::SdlApplication& app) override
    {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Tungsten::draw_triangle_elements_16(0, count_);
    }
private:
    std::shared_ptr<BitmapFont> bmp_font_;
    GlFont font_;
    std::u32string text_;
    std::vector<Tungsten::BufferHandle> buffers_;
    Tungsten::VertexArrayHandle vertex_array_;
    ShowTextShaderProgram program_;
    int32_t count_ = 0;
};

argos::ParsedArguments parse_arguments(int argc, char* argv[])
{
    argos::ArgumentParser parser(argv[0]);
    parser.about("Creates an OpenGL window where it displays a"
                 " given text with a given bitmap font.")
        .add(argos::Argument("TEXT")
                 .count(1, UINT_MAX)
                 .help("The text the program will display."))
        .add(argos::Option{"-b", "--bmpfont"}.argument("PATH")
                 .help("Path to a bitmap font. This can be either the"
                       " PNG file, the JSON file, or just the font name"
                       " without the extension."))
        .add(argos::Option{"-f", "--font"}.argument("FILE:SIZE")
                 .help("Path to a font (e.g. the .ttf file) and the size."));
    Tungsten::SdlApplication::add_command_line_options(parser);
    return parser.parse(argc, argv);
}

std::vector<char32_t> get_unique_chars(std::u32string_view str)
{
    std::unordered_set<char32_t> chars;
    for (auto ch : str)
        chars.insert(ch);
    return {chars.begin(), chars.end()};
}

int main(int argc, char* argv[])
{
    try
    {
        auto args = parse_arguments(argc, argv);
        auto texts = args.values("TEXT").as_strings();
        auto text8 = ystring::join(texts.begin(), texts.end(), " ");
        auto text32 = ystring::to_utf32(text8);
        auto chars = get_unique_chars(text32);

        auto bmp_font = std::make_shared<BitmapFont>();
        if (auto bmp_font_arg = args.value("--bmpfont"))
        {
            *bmp_font = read_bitmap_font(bmp_font_arg.as_string());
        }
        else if (auto font_arg = args.value("--font"))
        {
            auto parts = font_arg.split(':', 2, 2);
            *bmp_font = make_bitmap_font(parts.value(0).as_string(),
                                         parts.value(1).as_uint(),
                                         chars);
        }
        else
        {
            args.error("No font was specified.");
        }

        auto event_loop = std::make_unique<ShowText>(
            std::move(bmp_font),
            text32);
        Tungsten::SdlApplication app("ShowPng", std::move(event_loop));
        auto params = app.window_parameters();
        params.gl_parameters.multi_sampling = {1, 2};
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
