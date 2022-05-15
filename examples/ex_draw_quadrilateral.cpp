#define GL_SILENCE_DEPRECATION
#define SUPPORTS_VAO
#define MICROGL_USE_STD_MATH
#define NITROGL_ENABLE_THROW

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);
        glCheckError();
        canvas canva(500,500);
        auto tex_sampler_1 = texture_sampler(Resources::loadTexture("assets/images/test.png", true));
        auto tex_sampler_2 = texture_sampler(Resources::loadTexture("assets/images/test.png", false));
        auto tex_sampler_3 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", false));
        auto tex_sampler_4 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", true));
        color_sampler sampler_color(1.0,0.0,0.0,1.0/2);

        auto render = [&]() {
            static float t = 0;
            static float d =0;
            static float G = 400;
            d+=0.02;

            t+=0.5f;
            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawQuadrilateral(tex_sampler_3,
                                    0.0f,               0.0f,
                                    G + 100.0f + d,     0.0f,
                                    G + 0.0f,           G-20,
                                    0.0f,               G
                                    );

            glCheckError();
        };

        example_run(canva, render);
    };

    example_init(on_init);
}

