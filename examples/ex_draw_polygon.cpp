#define GL_SILENCE_DEPRECATION
#define SUPPORTS_VAO
#define MICROGL_USE_STD_MATH
#define NITROGL_ENABLE_THROW

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/test_sampler.h>
#include <nitrogl/samplers/mix_sampler.h>
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/shapes/circle_sampler.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

dynamic_array<vec2f> poly_hole() {
    using il = std::initializer_list<vec2f>;
    vec2f p0 = {100, 100};
    vec2f p1 = {300, 100};
    vec2f p2 = {300, 300};
    vec2f p3 = {100, 300};

    vec2f p4 = {150, 150};
    vec2f p7 = {150, 250};
    vec2f p6 = {250, 250};
    vec2f p5 = {250, 150};

    //    return {p4, p5, p6, p7};
    return il{p0, p1, p2, p3,   p4, p7, p6, p5, p4,p3};//,p5_,p4_};
}

dynamic_array<vec2f> poly_diamond() {
    using il = std::initializer_list<vec2f>;
    return il{{300, 100}, {400, 300}, {300, 400}, {100,300}};
}

dynamic_array<vec2f> poly_rect(float w, float h) {
    using il = std::initializer_list<vec2f>;
    return il{{0, 0}, {w, 0}, {w, h}, {0,h}};
}

dynamic_array<vec2f> poly_1_x_monotone() {
    using il = std::initializer_list<vec2f>;
    return il{
        {50,100},
        {100,50},
        {150,100},
        {200,50},
        {300,100},
        {400,50},
        {500,100},

        {500,200},
        {400,100+50},
        {300,100+100},
        {200,100+50},
        {150,100+100},
        {100,100+50},
        {50,100+100},

        };
}

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(100,200);
        glCheckError();
        canvas canva(600,600);
        auto tex_sampler_1 = texture_sampler(Resources::loadTexture("assets/images/test.png", true));
        auto tex_sampler_2 = texture_sampler(Resources::loadTexture("assets/images/test.png", false));
        auto tex_sampler_3 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", true), true);
        color_sampler sampler_color(0.0,0.0,0.0,1.0);

        const auto polygon_diamond = poly_diamond();
        const auto polygon_x_monotone = poly_1_x_monotone();
        const auto polygon_rect = poly_rect(100,200);
        const auto & polygon = polygon_x_monotone;
        auto render = [&]() {
            static float t = 0.0f;
            t+=0.00005f;

            canva.clear(1.0, 1.0, 1.0, 1.0);
//            canva.drawPolygon<polygons::CONVEX>(tex_sampler_3, polygon_rect.data(), polygon_rect.size());
//            canva.drawPolygon<polygons::SIMPLE>(tex_sampler_3, polygon_diamond.data(), polygon_diamond.size());
            canva.updateDrawMode(draw_mode::fill);
            canva.drawPolygon<polygons::X_MONOTONE>(tex_sampler_3,
                                                    polygon_x_monotone.data(),
                                                    polygon_x_monotone.size());
            canva.updateDrawMode(draw_mode::line);
            canva.drawPolygon<polygons::X_MONOTONE>(sampler_color,
                                                    polygon_x_monotone.data(),
                                                    polygon_x_monotone.size());
            glCheckError();
        };

        example_run<false>(canva, render);
    };

    example_init(on_init);
}

