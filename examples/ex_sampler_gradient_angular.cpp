#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/test_sampler.h>
#include <nitrogl/samplers/mix_sampler.h>
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/shapes/circle_sampler.h>
#include <nitrogl/samplers/gradients/angular_gradient.h>
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

        angular_gradient gradient { math::deg_to_rad(0.0f),
                                   math::deg_to_rad(360.0f) };
        float h = 1.0f/13;

        gradient.addStop(h*0, {0.5,1.0,0, 1});
        gradient.addStop(h*1, {1.0,1.0,0, 1});
        gradient.addStop(h*2, {1.0,0.5,0, 1});
        gradient.addStop(h*3, {1.0,0,0, 1});
        gradient.addStop(h*4, {1.0,0,0.5, 1});
        gradient.addStop(h*5, {1.0,0,1.0, 1});
        gradient.addStop(h*6, {0.5,0,1, 1});
        gradient.addStop(h*7, {0.0,0,1, 1});
        gradient.addStop(h*8, {0.0,0.5,1, 1});
        gradient.addStop(h*9, {0,1,1, 1});
        gradient.addStop(h*10, {0,1,0.5, 1});
        gradient.addStop(h*11, {0,1,0, 1});
        gradient.addStop(h*12, {0.5,1,0, 1});

//        gradient.addStop(0.0f, {1,0,0, 1});
//        gradient.addStop(1.0f, {0,1,0, 1});

        auto render = [&]() {
            static float t= 0;
            t+=0.01;

            // some cool animations
//            gradient.setNewRadial({0.5f, 0.5f}, (1.25f+std::sin(t/100.0f))/2.0f);
//            gradient.setNewRadial({std::sin(t/1.0f), 0.5f}, 0.5f);
//            gradient.updateStop(3, 1.0f, {1.0, 0.0, 1.0, (std::sin(t/10.0f)+1.0f)/2.0f});

            canva.clear(1.0, 1.0, 1.0, 1.0);
//            canva.drawRect(tex_sampler_3, 0, 0, 250, 250);//, mat3f::rotation(t));
            canva.drawRect(gradient, 0, 0, 250, 250, 1.0);
            canva.drawArc(gradient, sampler_color,
                                      300, 300,
                                      100,
                                      math::deg_to_rad(0.0f),
                                      math::deg_to_rad(t),
                                      10.0f, 1.0f,
                                      1.0);//,
//                          mat3f::rotation(nitrogl::math::deg_to_rad(t), 100,100));

        };

        example_run(canva, render);
    };

    example_init(on_init);
}

