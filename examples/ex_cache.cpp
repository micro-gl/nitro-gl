#define GL_SILENCE_DEPRECATION
#define SUPPORTS_VAO
#define MICROGL_USE_STD_MATH
#define NITROGL_ENABLE_THROW

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/traits.h>
#include <nitrogl/_internal/bits_robin_lru_cache.h>
#include <nitrogl/_internal/bits_robin_lru_pool.h>

using namespace nitrogl;

struct TT {
    uint32_t a; //4
    uint8_t b; // 1
    uint16_t c; // 2

};

void test_cache_2() {
    int bb = sizeof (long);
    //        bits_robin_lru_cache<4, unsigned int, std::allocator<char>> cache{0.125f};
    //        bits_robin_lru_cache<10, unsigned int, nitrogl::std_rebind_allocator<>> cache{0.125f};
    bits_robin_lru_cache<4, unsigned int, nitrogl::std_rebind_allocator<char>> cache{0.125f};

    //        for (int ix = 0; ix < 15*2; ++ix) {
    //            cache.put(ix, ix);
    //        }
    //

    //        cache.print(1, 10);
    //
    //        return;
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


}

void test_cache_1() {
    int bb = sizeof (long);
    //        bits_robin_lru_cache<4, unsigned int, std::allocator<char>> cache{0.125f};
    //        bits_robin_lru_cache<10, unsigned int, nitrogl::std_rebind_allocator<>> cache{0.125f};
    bits_robin_lru_pool<4, unsigned int, nitrogl::std_rebind_allocator<char>> cache{0.0f};
    cache.print();

//    for (int ix = 0; ix < 15*2; ++ix) {
//        cache.get(ix);
//        cache.print();
//     }
//    cache.get(28);
//    cache.print();


    //
    //        return;
    cache.get(0);
    cache.get(1);
    cache.get(2);
    cache.get(3);
    cache.print();
    cache.get(16);
    cache.print();
    cache.get(32);
    cache.print();
    cache.get(33);
    cache.print();
    //        cache.remove(0);
    cache.print();
    cache.remove(32);
    cache.print();
    cache.remove(16);
    cache.print();
    cache.get(13);
    cache.get(13+16);
    cache.print();
    cache.remove(13);
    cache.print();
    cache.clear();
    cache.print();


}

int main() {

    auto on_init = [](SDL_Window *, void *) {
        glCheckError();

        test_cache_1();
    };

    example_init(on_init);
}

