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
        lru_cache<4, int> cache;

        //
//        cache.print();
//        cache.put(0, 0);
//        cache.put(16, 1);
//        cache.put(32, 2);
//        cache.print();
//        cache.remove(0);
//        cache.print();
//
//        return;
//        //


        cache.print();
        cache.put(0, 0);
        cache.put(1, 1);
        cache.put(2, 2);
        cache.put(3, 3);
        cache.print();
        cache.put(16, 10);
        cache.print();
        cache.put(32, 10);
        cache.print();
        cache.put(33, 12);
        cache.print();
//        cache.remove(0);
        cache.print();
        cache.remove(32);
        cache.print();
        cache.remove(16);
        cache.print();
        cache.put(13, 13);
        cache.put(13+16, 13);
        cache.print();
        cache.remove(13);
        cache.print();

    };

    example_init(on_init);
}

