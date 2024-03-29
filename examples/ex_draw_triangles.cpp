#define NITROGL_OPENGL_MAJOR_VERSION 4
#define NITROGL_OPENGL_MINOR_VERSION 1
//#define NITROGL_OPEN_GL_ES

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/shapes/circle_sampler.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);

        canvas canva(500,500);
        auto tex_sampler_1 = texture_sampler(Resources::loadTexture("assets/images/test.png", true));
        auto tex_sampler_2 = texture_sampler(Resources::loadTexture("assets/images/test.png", false));
        auto tex_sampler_3 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", true));
        color_sampler sampler_color(1.0,0.0,0.0,1.0/2);

        const float SIZE = 200;
        vec2f vertices[4] = {
                {0, 0},
                {SIZE, 0},
                {SIZE, SIZE},
                {0, SIZE}
        };

        auto render_triangles = [&]() {
            static float t = 0.0f;
            t+=0.00005f;

            static const unsigned int indices[6] = { 0, 1, 2, 2, 3, 0 } ;
            static const auto type = nitrogl::triangles::indices::TRIANGLES;

            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawTriangles(tex_sampler_3,
                                type,
                                vertices, 4,
                                indices, 6,
                                nullptr, 0,
                                mat3f::rotation(t, SIZE/2.0f, SIZE/2.0f).post_translate(vec2f {200, 200})
                                );
        };

        auto render_triangles_fan = [&]() {
            static float t = 0.0f;
            t+=0.00005f;

            static const unsigned int indices[4] = { 0, 1, 2, 3 } ;
            static const auto type = nitrogl::triangles::indices::TRIANGLES_FAN;

            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawTriangles(tex_sampler_3,
                                type,
                                vertices, 4,
                                indices, 4,
                                nullptr, 0,
                                mat3f::rotation(t, SIZE/2.0f, SIZE/2.0f).post_translate(vec2f {200, 200})
                                );
        };

        auto render_triangles_strip = [&]() {
            static float t = 0.0f;
            t+=0.00005f;

            static const unsigned int indices[4] = { 0, 1, 3, 2 } ;
            static const auto type = nitrogl::triangles::indices::TRIANGLES_STRIP;

            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawTriangles(tex_sampler_3,
                                type,
                                vertices, 4,
                                indices, 4,
                                nullptr, 0,
                                mat3f::rotation(t, SIZE/2.0f, SIZE/2.0f).post_translate(vec2f {200, 200})
                                );
        };

        example_run<true>(canva, render_triangles_strip);
    };

    example_init(on_init);
}

