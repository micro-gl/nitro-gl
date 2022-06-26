#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/shapes/circle_sampler.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);

        canvas canva(500,500);
        auto tex_sampler_1 = texture_sampler(Resources::loadTexture("assets/images/test.png", true));
        auto tex_sampler_2 = texture_sampler(Resources::loadTexture("assets/images/test.png", false));
        auto tex_sampler_3 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", true));
        color_sampler sampler_color(1.0,0.0,0.0,1.0/2);
//        circle_sampler circle { 0.45f, 0.01f, 0.01f, &tex_sampler_3, &tex_sampler_3 };

//        circle.sub_samplers()[0] = &tex_sampler_3;

        auto render = [&]() {
            static float t= 0;
            t+=0.005;
            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawRoundedRect(tex_sampler_3, sampler_color,
                                  100, 100, 500,  500,
                                  50, 1., 1.0,
                                  mat3f::rotation(nitrogl::math::deg_to_rad(t), 200,200));
            glCheckError();
        };

        example_run(canva, render);
    };

    example_init(on_init);
}

