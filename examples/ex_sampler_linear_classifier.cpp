#define GL_SILENCE_DEPRECATION
#define SUPPORTS_VAO
#define MICROGL_USE_STD_MATH
#define NITROGL_ENABLE_THROW

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/test_sampler.h>
#include <nitrogl/samplers/mix_sampler.h>
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/shapes/linear_classifier_sampler.h>
#include <nitrogl/canvas.h>
#include <nitrogl/functions/lerp.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);
        glCheckError();
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
            glCheckError();
        };

        example_run(canva, render);
    };

    example_init(on_init);
}

