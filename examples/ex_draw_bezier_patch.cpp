#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

float * bi_cubic_1(){
    static float geo[4 * 4 * 2] {
        // row 0
        1.0f,     0.0f,
        170.66f,  0.0f,
        341.333f, 0.0f,
        512.0f,   0.0f,

        // row 1
        1.0f,     170.66f,
        293.44f,  162.78f,
        746.68f,  144.65f,
        512.0f,   170.66f,

        // row 2
        1.0f,     341.33f,
        383.12f,  327.69f,
        1042.79f, 296.31f,
        512.0f,   341.33f,

        // row 3
        1.0f,     512.0f,
        170.66f,  512.0f,
        341.333f, 512.0f,
        512.0f,   512.0f,
    };

    return geo;
}

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);

        canvas canva(600,600);
        auto tex_sampler_1 = texture_sampler(Resources::loadTexture("assets/images/test.png", true));
        auto tex_sampler_2 = texture_sampler(Resources::loadTexture("assets/images/test.png", false));
        auto tex_sampler_3 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", true), false);
        color_sampler sampler_color(1.0,0.0,0.0,1.0/2);

        auto render = [&]() {
            static float t = 0.0f;
            t+=0.005;

            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawBezierPatch<microtess::patch_type::BI_CUBIC>(
                    tex_sampler_3,
                    bi_cubic_1(),
                    20, 20);
        };

        example_run<true>(canva, render);
    };

    example_init(on_init);
}

