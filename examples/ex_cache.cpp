#define GL_SILENCE_DEPRECATION
#define SUPPORTS_VAO
#define MICROGL_USE_STD_MATH
#define NITROGL_ENABLE_THROW

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/_internal/lru_cache.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        glCheckError();


    };

    example_init(on_init);
}

