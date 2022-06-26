#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/block_sampler.h>
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/color_sampler.h>
#include <nitrogl/samplers/d1_function_sampler.h>
#include <nitrogl/samplers/block_sampler.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);

        canvas canva(500,500);
        auto tex_sampler_1 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", true));
        auto tex_sampler_2 = texture_sampler(Resources::loadTexture("assets/images/test.png", true));
        color_sampler black {0.0f, 0.0f, 0.0f, 1.0f};
        d1_function_sampler sampler(&tex_sampler_1, &black);
        block_sampler block_sampler(&sampler, 16);

        const unsigned size = 100;
        vec2f points[size];

        auto render = [&]() {
            static float t= 0;
            t+=0.0001;
            for (int ix = 0; ix < size; ++ix) {
                float xx = float(ix)/(size-1);
                float yy = nitrogl::math::sin(t+ xx*1.0f*nitrogl::math::two_pi<float>())*0.25f+0.5f;
                vec2f v {xx, yy};
                points[ix]=v;
            }

            sampler.update_points(points, size/1);

            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawRect(sampler, 0, 0, 256, 256);
            canva.drawRect(block_sampler, 256, 256, 256+128, 256+128);
        };

        example_run<>(canva, render);
    };

    example_init(on_init);
}

