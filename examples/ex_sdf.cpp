#define GL_SILENCE_DEPRECATION
#define SUPPORTS_VAO
#define MICROGL_USE_STD_MATH
#define NITROGL_ENABLE_THROW

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/test_sampler.h>
#include <nitrogl/samplers/mix_sampler.h>
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/sdf_sampler.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);
        glCheckError();
//        canvas canva(tex);
        canvas canva(500,500);
        auto tex_sampler_1 = texture_sampler<true>(Resources::loadTexture("assets/images/test.png", true));
        auto tex_sampler_2 = texture_sampler<true>(Resources::loadTexture("assets/images/test.png", false));
        auto tex_sampler_3 = texture_sampler<>(Resources::loadTexture("assets/images/uv_256.png", true));
        sdf_sampler sdf;
        color_sampler sampler_color(1.0,0.0,0.0,1.0);

//        glEnable(GL_BLEND);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//        glBlendFunc(GL_ONE, GL_ZERO);
        auto render = [&]() {
            static float t= 0;
            t+=0.05;
            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawRect2(sdf, 0, 0, 250, 250);
//            canva.drawRect(tex_sampler_3, 125, 125, 125+250, 125+250);//, mat3f::rotation(t));
            glCheckError();
        };

        example_run(canva, render);
    };

    example_init(on_init);
}

