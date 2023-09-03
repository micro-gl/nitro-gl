#define NITROGL_OPENGL_MAJOR_VERSION 4
#define NITROGL_OPENGL_MINOR_VERSION 1
//#define NITROGL_OPEN_GL_ES

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/shapes/circle_sampler.h>
#include <nitrogl/samplers/gradients/circular_gradient.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);

        canvas canva(500,500);
        auto tex_sampler_1 = texture_sampler(Resources::loadTexture("assets/images/test.png", true));
        auto tex_sampler_2 = texture_sampler(Resources::loadTexture("assets/images/test.png", false));
        auto tex_sampler_3 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", true));

        circular_gradient gradient{{0.5f,0.5f}, 0.5f};

        gradient.addStop(0.0f, {1,0,0, 1});
        gradient.addStop(0.45f, {1,0,0, 1});
        gradient.addStop(0.50f, {0,1,0, 1});
        gradient.addStop(1.f, {1,0,1, 1});

        auto render = [&]() {
            static float t= 0;
            t+=0.05;

            // some cool animations
//            gradient.setNewRadial({0.5f, 0.5f}, (1.25f+std::sin(t/100.0f))/2.0f);
//            gradient.setNewRadial({std::sin(t/1.0f), 0.5f}, 0.5f);
//            gradient.updateStop(3, 1.0f, {1.0, 0.0, 1.0, (std::sin(t/10.0f)+1.0f)/2.0f});

            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawRect(tex_sampler_3, 0, 0, 250, 250);//, mat3f::rotation(t));
            canva.drawRect(gradient, 0, 0, 250, 250, 1.0);
        };

        example_run(canva, render);
    };

    example_init(on_init);
}

