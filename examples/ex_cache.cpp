#include <nitrogl/traits.h>
#include <nitrogl/_internal/bits_robin_lru_cache.h>
#include <nitrogl/_internal/bits_robin_lru_pool.h>
#include <nitrogl/_internal/bits_linear_probe_lru_pool.h>

using namespace nitrogl;

struct TT {
    unsigned int a; //4
    unsigned char b; // 1
};

void test_cache_linear_probe() {
    int bb = sizeof (TT);
    //        bits_robin_lru_cache<4, unsigned int, std::allocator<char>> cache{0.125f};
    //        bits_robin_lru_cache<10, unsigned int, nitrogl::std_rebind_allocator<>> cache{0.125f};
    microc::bits_linear_probe_lru_pool<4, unsigned int, nitrogl::std_rebind_allocator<char>> pool{0.5f};

    pool.print();

    for (int ix = 0; ix < 15*2; ++ix) {
        pool.get_or_put(ix);
        pool.print();
     }
    pool.print();
    pool.get(28);
    pool.print();

    return;

    pool.get(0);
    pool.get(1);
    pool.get(2);
    pool.get(3);
    pool.print();
    pool.get(16);
    pool.print();
    pool.get(32);
    pool.print();
    pool.get(33);
    pool.print();
    //        pool.remove(0);
    pool.print();
    pool.remove(32);
    pool.print();
    pool.remove(16);
    pool.print();
    pool.get(13);
    pool.get(13 + 16);
    pool.print();
    pool.remove(13);
    pool.print();
    pool.get(33);
    pool.print();
    pool.get(5);
    pool.print();
    pool.clear();
    pool.print();


}

void test_cache_robin_hood() {
    int bb = sizeof (long);
    //        bits_robin_lru_cache<4, unsigned int, std::allocator<char>> pool{0.125f};
    //        bits_robin_lru_cache<10, unsigned int, nitrogl::std_rebind_allocator<>> pool{0.125f};
    microc::bits_robin_lru_pool<4, unsigned int, nitrogl::std_rebind_allocator<char>> pool{0.5f};
    pool.print();

//    for (int ix = 0; ix < 15*2; ++ix) {
//        pool.get(ix);
//        pool.print();
//     }
//    pool.get(28);
//    pool.print();

    //
    //        return;
    pool.get_or_put(0);
    pool.get_or_put(1);
    pool.get_or_put(2);
    pool.get_or_put(3);
    pool.print();
    pool.get_or_put(16);
    pool.print();
    pool.get_or_put(32);
    pool.print();
    pool.get_or_put(33);
    pool.print();
    //        pool.remove(0);
    pool.print();
    pool.remove(32);
    pool.print();
    pool.remove(16);
    pool.print();
    pool.get_or_put(13);
    pool.get_or_put(13 + 16);
    pool.print();
    pool.remove(13);
    pool.print();
    pool.clear();
    pool.print();


}

int main() {

//    test_cache_linear_probe();
    test_cache_robin_hood();
    return 0;
}

