#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/color_sampler.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);

        canvas canva(500,500);
        color_sampler sampler_color(1.0,0.0,1.0,1.0);

        auto render = [&]() {
            static float t= 0;
            t+=0.05;
            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawRect(sampler_color, 0, 0, 250, 250);
        };

        example_run(canva, render);
    };

    example_init(on_init);
}

