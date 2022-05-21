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
#include <nitrogl/samplers/gradients/line_gradient.h>
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

        line_gradient gradient{{0,1}, {1, 0}};

        gradient.addStop(0.0f, {1.0,0,0, 1.0});
        gradient.addStop(0.5f, {0,1.0,0, 1.0});
        gradient.addStop(1.f, {0,0,1.0, 1.0});

        auto render = [&]() {
            static float t= 0;
            t=0.05;

//            gradient.updateStop(1, 0.5f, {0.0, 1.0, 0.0, (std::sin(t/10.0f)+1.0f)/2.0f});
            gradient.rotate(0.0006f);
            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawRect(tex_sampler_3, 0, 0, 250, 250);//, mat3f::rotation(t));
            canva.drawRect(gradient, 0, 0, 250, 250, 1.0);
//            canva.drawRect(circle, 0, 0, 250, 250, 1.0/2, mat3f::rotation(t, 125, 125));
            glCheckError();
        };

        example_run(canva, render);
    };

    example_init(on_init);
}
