#define GL_SILENCE_DEPRECATION
#include "src/Resources.h"
#include "src/example.h"
#include <nitrogl/ogl/ebo.h>
#include <nitrogl/ogl/vbo.h>
#include <nitrogl/ogl/vao.h>
#include <nitrogl/ogl/fbo.h>
#include <nitrogl/ogl/gl_texture.h>

struct CC {
    int width() { return 600; }
    int height() { return 600; }
};
int main() {

    const auto render = [](void *, void *, void *) {
    };

    example_run(nullptr, render);
}

