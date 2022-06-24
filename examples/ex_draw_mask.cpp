#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/masking_sampler.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);
        canvas canva(500,500);
        auto tex_sampler_1 = texture_sampler(Resources::loadTexture("assets/images/test.png", true));
        auto tex_sampler_2 = texture_sampler(Resources::loadTexture("assets/images/test.png", false));
        auto tex_sampler_3 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", true));
        auto tex_sampler_4 = texture_sampler(Resources::loadTexture("assets/images/dog_32bit_premul.png", true));
        auto tex_sampler_5 = texture_sampler(Resources::loadTexture("assets/images/bw_8bits.png", false));
        color_sampler sampler_color(1.0,0.0,0.0,1.0);

        auto render = [&]() {
            static float t= 0;
            t+=0.05;
            canva.clear(1.0, 1.0, 1.0, 1.0);
            for (int ix = 0; ix < 15; ++ix)
            canva.drawRect(tex_sampler_3, 0, 0, 500, 500, 1.0);
            canva.drawMask(tex_sampler_4, channels::channel::alpha_channel_inverted,
                           0, 0, 250, 250);
            canva.drawMask(tex_sampler_4, channels::channel::alpha_channel,
                           250, 0, 500, 250);
            canva.drawMask(tex_sampler_5, channels::channel::red_channel_inverted,
                           0, 250, 250, 500);
            canva.drawMask(tex_sampler_5, channels::channel::red_channel,
                           250, 250, 500, 500);
        };

        example_run(canva, render);
    };

    example_init(on_init);
}

