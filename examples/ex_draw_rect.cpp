#define GL_SILENCE_DEPRECATION
#define SUPPORTS_VAO
#define MICROGL_USE_STD_MATH
#define NITROGL_ENABLE_THROW

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/test_sampler.h>
#include <nitrogl/samplers/mix_sampler.h>
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/shapes/circle_sampler.h>
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
        color_sampler sampler_color(1.0,0.0,0.0,1.0/2);

        auto render = [&]() {
            static float t = 0.0f;
            t+=0.005;
            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawRect(tex_sampler_3,
                           100, 100, 200,  200,
                           1.0f,
                           mat3f::rotation(nitrogl::math::deg_to_rad(t), 50,50));
            glCheckError();
        };

        example_run<true>(canva, render);
    };

    example_init(on_init);
}

