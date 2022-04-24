#define GL_SILENCE_DEPRECATION
#define SUPPORTS_VAO
#define MICROGL_USE_STD_MATH
#define NITROGL_ENABLE_THROW

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/test_sampler.h>
#include <nitrogl/samplers/mix_sampler.h>
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/circle_sampler.h>
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
        color_sampler sampler_color(1.0,0.0,0.0,1.0/2);
//        circle_sampler circle { 0.45f, 0.01f, 0.01f, &tex_sampler_3, &tex_sampler_3 };

//        circle.sub_samplers()[0] = &tex_sampler_3;

        auto render = [&]() {
            static float t= 0;
            t+=0.05;
            canva.clear(1.0, 1.0, 1.0, 1.0);
//            canva.drawCircle(tex_sampler_3, sampler_color, 250, 250, 200, 10., 1.0);
            canva.drawCircle(tex_sampler_3, sampler_color,
                             200, 200,
                             100, 70.,
                             1.0,
                             mat3f::rotation(nitrogl::math::deg_to_rad(t), 100,100));
            glCheckError();
        };

        example_run(canva, render);
    };

    example_init(on_init);
}

