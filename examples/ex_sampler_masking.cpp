#define GL_SILENCE_DEPRECATION
#define NITROGL_SUPPORTS_VAO
#define NITROGL_USE_STD_MATH
#define NITROGL_DISABLE_THROW

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/test_sampler.h>
#include <nitrogl/samplers/mix_sampler.h>
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/masking_sampler.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);
        glCheckError();
        canvas canva(500,500);
        auto tex_sampler_1 = texture_sampler(Resources::loadTexture("assets/images/test.png", true));
        auto tex_sampler_2 = texture_sampler(Resources::loadTexture("assets/images/test.png", false));
        auto tex_sampler_3 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", true));
        auto tex_sampler_4 = texture_sampler(Resources::loadTexture("assets/images/dog_32bit_premul.png", true));
        auto tex_sampler_5 = texture_sampler(Resources::loadTexture("assets/images/bw_8bits.png", false));
        color_sampler sampler_color(1.0,0.0,0.0,1.0);
        masking_sampler sampler_1 {&tex_sampler_3, &tex_sampler_4 };
        masking_sampler sampler_2 {&tex_sampler_3, &tex_sampler_5, channels::channel::red_channel };

        auto render = [&]() {
            static float t= 0;
            t+=0.05;
            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawRect(sampler_1, 0, 0, 250, 250, 1.0);
            canva.drawRect(sampler_2, 250, 250, 500, 500, 1.0);
            glCheckError();
        };

        example_run(canva, render);
    };

    example_init(on_init);
}

