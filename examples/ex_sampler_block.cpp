#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/block_sampler.h>
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);

        canvas canva(500,500);
        auto tex_sampler_1 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", true));
        auto tex_sampler_2 = texture_sampler(Resources::loadTexture("assets/images/test.png", true));
        block_sampler block_sampler(&tex_sampler_1, 16);

        auto render = [&]() {
            static float t= 0;
            t+=0.05;
            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawRect(block_sampler, 0, 0, 256, 256);
        };

        example_run(canva, render);
    };

    example_init(on_init);
}

