#define GL_SILENCE_DEPRECATION
#define SUPPORTS_VAO
#include "src/Resources.h"
#include "src/example.h"
#include <nitrogl/ogl/ebo.h>
#include <nitrogl/ogl/vbo.h>
#include <nitrogl/ogl/vao.h>
#include <nitrogl/ogl/fbo.h>
#include <nitrogl/ogl/gl_texture.h>
#include <nitrogl/ogl/shader.h>
#include <nitrogl/ogl/shader_program.h>
#include <nitrogl/ogl/main_shader_program.h>
#include <nitrogl/ogl/render_node.h>
#include <nitrogl/ogl/main_render_node.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

struct CC {
    int width() { return 600; }
    int height() { return 600; }
};


int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture::empty(500,500);
        glCheckError();
        canvas canva(tex);
//        canvas canva(500,500);

        auto render = [&]() {
            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawRect(1.0, 1.0, 1.0, 1.0);
        };

        example_run(canva, render);
    };

    example_init(on_init);
}

