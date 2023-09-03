#define NITROGL_OPENGL_MAJOR_VERSION 4
#define NITROGL_OPENGL_MINOR_VERSION 1
//#define NITROGL_OPEN_GL_ES

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/shapes/rounded_rect_sampler.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);

        canvas canva(500,500);
        auto tex_sampler_1 = texture_sampler(Resources::loadTexture("assets/images/test.png", true));
        auto tex_sampler_2 = texture_sampler(Resources::loadTexture("assets/images/test.png", false));
        auto tex_sampler_3 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", true));
        color_sampler sampler_color(1.0,0.0,0.0,1.0);
        rounded_rect_sampler sampler { &tex_sampler_3, &sampler_color,
                                       0.15, .9, 0.00,
                                       0.03f,
                                       0.00f, 0.01f };

        auto render = [&]() {
            static float t= 0;
            t+=0.05;
            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawRect(sampler, 0, 0, 250, 250, 1.0);
//            canva.drawRect(circle, 0, 0, 250, 250, 1.0/2, mat3f::rotation(t, 125, 125));
//            canva.drawRect(tex_sampler_3, 125, 125, 125+250, 125+250);//, mat3f::rotation(t));
        };

        example_run(canva, render);
    };

    example_init(on_init);
}

