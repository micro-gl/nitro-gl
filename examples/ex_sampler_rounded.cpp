#define GL_SILENCE_DEPRECATION
#define SUPPORTS_VAO
#define MICROGL_USE_STD_MATH
#define NITROGL_ENABLE_THROW

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/test_sampler.h>
#include <nitrogl/samplers/mix_sampler.h>
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/rounded_rect_sampler.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);
        glCheckError();
        canvas canva(500,500);
        auto tex_sampler_1 = texture_sampler<true>(Resources::loadTexture("assets/images/test.png", true));
        auto tex_sampler_2 = texture_sampler<true>(Resources::loadTexture("assets/images/test.png", false));
        auto tex_sampler_3 = texture_sampler<false>(Resources::loadTexture("assets/images/uv_256.png", true));
        color_sampler sampler_color(1.0,0.0,0.0,1.0);
        rounded_rect_sampler sampler { 0.15, .9, 0.00, 0.03f, 0.00f, 0.01f, &tex_sampler_3, &sampler_color };

        auto render = [&]() {
            static float t= 0;
            t+=0.05;
            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawRect(sampler, 0, 0, 250, 250, 1.0);
//            canva.drawRect(circle, 0, 0, 250, 250, 1.0/2, mat3f::rotation(t, 125, 125));
//            canva.drawRect(tex_sampler_3, 125, 125, 125+250, 125+250);//, mat3f::rotation(t));
            glCheckError();
        };

        example_run(canva, render);
    };

    example_init(on_init);
}

