#define GL_SILENCE_DEPRECATION
#define SUPPORTS_VAO
#define MICROGL_USE_STD_MATH
#define NITROGL_ENABLE_THROW

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/ogl/ebo.h>
#include <nitrogl/ogl/vbo.h>
#include <nitrogl/ogl/vao.h>
#include <nitrogl/ogl/fbo.h>
#include <nitrogl/ogl/gl_texture.h>
#include <nitrogl/ogl/shader.h>
#include <nitrogl/ogl/shader_program.h>
#include <nitrogl/_internal/main_shader_program.h>
#include <nitrogl/render_nodes/multi_render_node.h>
#include <nitrogl/samplers/test_sampler.h>
#include <nitrogl/samplers/mix_sampler.h>
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/_internal/shader_compositor.h>
//#include <nitrogl/ogl/typed_shader_program.h>
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
        mix_sampler sampler_mix;
        color_sampler sampler_color(1.0,0.0,0.0,1.0);

        glEnable(GL_BLEND);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_ONE, GL_ZERO);
        auto render = [&]() {
            static float t= 0;
            t+=0.05;
            canva.clear(1.0, 1.0, 1.0, 1.0);
//            canva.drawRect(100, 100, 150, 150,
//                           mat3f::rotation(math::deg_to_rad(t), 25, 25));
            canva.drawRect(tex_sampler_1, 0, 0, 250, 250);
            canva.drawRect(sampler_mix, 250, 0, 500, 250);
            canva.drawRect(tex_sampler_3, 125, 125, 125+250, 125+250);//, mat3f::rotation(t));
//            canva.drawRect(0, 125, 500, 260);
//canva.drawRect(0, 250, 250, 270,
//                           mat3f::rotation(math::deg_to_rad(t), 125, 10));
            glCheckError();
//            canva.drawRect(50, 100, 200, 200);
        };

        example_run(canva, render);
    };

    example_init(on_init);
}

