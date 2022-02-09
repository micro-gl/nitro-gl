#define GL_SILENCE_DEPRECATION
#define SUPPORTS_VAO
#define MICROGL_USE_STD_MATH
#define NITROGL_ENABLE_THROW

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/traits.h>
#include <nitrogl/_internal/lru_cache.h>

using namespace nitrogl;

struct TT {
    uint32_t a; //4
    uint8_t b; // 1
    uint16_t c; // 2

};

int main() {

    auto on_init = [](SDL_Window *, void *) {
        glCheckError();
        int bb = sizeof (long);
//        lru_cache<4, unsigned int, std::allocator<char>> cache{0.125f};
//        lru_cache<10, unsigned int, nitrogl::std_rebind_allocator<>> cache{0.125f};
        lru_cache<21, unsigned long, nitrogl::std_rebind_allocator<char>> cache{0.125f};

        for (int ix = 0; ix < 15*2; ++ix) {
            cache.put(ix, ix);
        }

        //
        cache.print(1, 10);

        return;
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
        cache.clear();
        cache.print();



    };

    example_init(on_init);
}

