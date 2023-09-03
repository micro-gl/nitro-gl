#define NITROGL_OPENGL_MAJOR_VERSION 4
#define NITROGL_OPENGL_MINOR_VERSION 1
//#define NITROGL_OPEN_GL_ES

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/shapes/linear_classifier_sampler.h>
#include <nitrogl/canvas.h>
#include <nitrogl/functions/lerp.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);

        canvas canva(500,500);
        auto tex_sampler_1 = texture_sampler(Resources::loadTexture("assets/images/test.png", true));
        auto tex_sampler_2 = texture_sampler(Resources::loadTexture("assets/images/test.png", false));
        auto tex_sampler_3 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", true));
        color_sampler color_a {0.0,1.0,0.0,1.0};
        color_sampler color_b (1.0,0.0,1.0,1.0);
        linear_classifier_sampler sampler { &tex_sampler_3, &color_b,
                                  {0.0f, 0.0f}, {1.0f, 1.0f},
                                  0.050f};

        auto render = [&]() {
            static float t= 0;

            t+=0.0005;

            float alpha = nitrogl::math::abs(nitrogl::math::sin(t));

            sampler.p1 = vec2f (1.0f, alpha);

            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawRect(sampler, 0, 0, 250, 250, 1.0);
        };

        example_run(canva, render);
    };

    example_init(on_init);
}

