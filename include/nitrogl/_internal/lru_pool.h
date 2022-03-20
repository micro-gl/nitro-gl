/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include "bits_linear_probe_lru_pool.h"
#include "bits_robin_lru_pool.h"

namespace microc {
#define LRU_PRINT_SEQ 0
#define LRU_PRINT_ORDER_MRU 1
#define LRU_PRINT_ORDER_FREE_LIST 2
#define LRU_CACHE_ALLOW_PRINT

#ifdef LRU_CACHE_ALLOW_PRINT
#include <iostream>
#endif
    /**
     * LRU object Pool with constrained bits:
     * 1. size is upto 10 bits = 1024 for 32 bits keys
     * 2. size is upto 21 bits = 2,097,152 for 64 bits keys
     * 3. compact lookup and is perfect for CPU cache
     * 4. Objects can be constructed at init time, and are never destructed (but rather reused), unless the pool is destructed
     * 5. You can also defer construction to a later time.
     * This uses a hash table with robin hood probing and in-place linked-list, that fits in a machine word.
     * and is very conservative with memory, which allows CPU caches to load many entries at once.
     *
     * NOTES:
     * - Keys are assumed to be unique (if you are using hash function as a source for keys use a good one)
     * - for 32 bits keys, each lru key + list entry is 64 bit.
     * - for 64 bits keys, each lru key + list entry is 128 bit.
     * - The size of the cache is a power of 2 of the bits for value, which gives some optimizations.
     * - perfect for small caches: up to 1024 entries for 32 bit keys and 2,097,152 for 64 bit keys.
     *
     * @tparam object_type The object type value to store
     * @tparam size_bits size of cache. 10 --> 2^10=1024 entries
     * @tparam machine_word the machine word type = short, int or long for key
     */
    template<class object_type, int size_bits=10,
            class machine_word=long, class Allocator=void>
    class lru_pool {
    private:
        using pool_t = bits_robin_lru_pool<size_bits, machine_word, Allocator>;
        using _pool_iter = typename pool_t::const_iterator;

        template<class iter_value_type> struct iterator_t {
            const lru_pool * _c; // container
            _pool_iter _i; // index

            static lru_pool * ncn(const lru_pool * node)
            { return const_cast<lru_pool *>(node); }
            iterator_t(_pool_iter i, const lru_pool * c) : _i(i), _c(c) {}
            template<class value_type_t>
            iterator_t(const iterator_t<value_type_t> & o) : iterator_t(o._i, o._c) {}
            iterator_t& operator++() {
                ++_i;
                return *this;
            }
            iterator_t& operator--() {
                --_i;
                return *this;
            }
            iterator_t operator+(int val) {
                iterator_t temp(*this);
                for (int ix = 0; ix < val; ++ix) ++temp;
                return temp;
            }
            iterator_t operator++(int) { iterator_t ret(_i, _c); ++(*this); return ret; }
            iterator_t operator--(int) { iterator_t ret(_i, _c); --(*this); return ret; }
            bool operator==(iterator_t o) const { return _i==o._i; }
            bool operator!=(iterator_t o) const { return !(*this==o); }
            iter_value_type operator*() const {
                const auto kv = *_i;
                return { kv.key, ncn(_c)->_items[kv.value] };
            }
        };

    public:
        using value_type = object_type;
        using size_type = int;
        using allocator_type = Allocator;
        using val_alloc = typename allocator_type::template rebind<value_type>::other;

        struct pair { machine_word key; value_type & value; };
        struct const_pair { machine_word key; const value_type & value; };

        using iterator = iterator_t<pair>;
        using const_iterator = iterator_t<const_pair>;
        iterator begin() noexcept { return iterator(_pool.begin(), this); }
        iterator end() noexcept { return iterator(_pool.end(), this); }
        const_iterator begin() const noexcept { return const_iterator(_pool.begin(), this); }
        const_iterator end() const noexcept { return const_iterator(_pool.end(), this); }

    private:
        pool_t _pool;
        value_type * _items;
        val_alloc _allocator;
        bool _are_items_constructed;

    public:
        struct result_type {
            object_type & object;
            // is the object the match we wanted or a new object from the pool.
            // If it is active, we can use it as is. If it is not, we need to
            // reconfigure it to use it.
            bool is_active;
        };

        template<class ...Args>
        explicit lru_pool(float load_factor=0.5f,
                 const allocator_type & allocator = allocator_type(),
                 Args && ...args) :
                 _pool(load_factor, allocator), _allocator(allocator), _items(nullptr),
                 _are_items_constructed(false) {
            _items = _allocator.allocate(_pool.capacity());
            construct(microc::traits::forward<Args>(args)...);
        }
        explicit lru_pool(float load_factor=0.5f,
                          const allocator_type & allocator = allocator_type()) :
                          _pool(load_factor, allocator), _allocator(allocator), _items(nullptr),
                          _are_items_constructed(false) {
            _items = _allocator.allocate(_pool.capacity());
        }

        ~lru_pool() {
            clear();
            destruct();
            _allocator.deallocate(_items);
            _items=nullptr;
        }

        allocator_type get_allocator() { return _allocator; }
        constexpr int capacity() const { return _pool.capacity(); }
        int size() const { return _pool.size(); }
        int maxSize() const { return _pool.maxSize(); }
        void print(char order=1, int how_many=-1) { _pool.print(order, how_many); }

        bool has(machine_word key) const { return _pool.has(key); }
        result_type get(machine_word key) {
            const auto q = _pool.get_or_put(key);
            return { _items[q.value], q.is_active };
        }
        void destruct() {
            if(!_are_items_constructed) return;
            for (int ix = 0; ix < capacity(); ++ix)
                (_items + ix)->~value_type();
            _are_items_constructed=false;
        }

        template<class ...Args>
        void construct(Args && ...args) {
            if(_are_items_constructed) destruct();
            for (int ix = 0; ix < capacity(); ++ix)
                ::new(_items + ix, microc_new::blah) value_type(microc::traits::forward<Args>(args)...);
            _are_items_constructed=true;
        }
        void clear() {
            // clear/reset all active values
            _pool.clear();
        }
    };

}