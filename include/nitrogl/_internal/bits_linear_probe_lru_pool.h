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

namespace microc {
#define LRU_PRINT_SEQ 0
#define LRU_PRINT_ORDER_MRU 1
#define LRU_PRINT_ORDER_FREE_LIST 2
#define LRU_CACHE_ALLOW_PRINT

#ifdef LRU_CACHE_ALLOW_PRINT
#include <iostream>
#endif
    /**
     * LRU Cache and pool for integer values with constrained bits:
     * 1. upto 10 bits per value for 32 bits keys
     * 2. upto 20 bits per value for 64 bits keys
     * This uses a hash table with linear probing and in-place linked-list,
     * and is very conservative with memory, which allows CPU caches to load many entries at once.
     *
     * NOTES:
     * - Keys are assumed to be unique (if you are using hash function as a source for keys use a good one)
     * - for 32 bits keys, each entry is 64 bit.
     * - for 64 bits keys, each entry is 128 bit.
     * - The size of the cache is a power of 2 of the bits for value, which gives some optimizations.
     * - perfect for small caches: up to 1024 entries for 32 bit keys and 1,048,576 for 64 bit keys.
     * - perfect for storing integer indices.
     *
     * @tparam size_bits the integer bits size
     * @tparam machine_word the machine word type = short, int or long
     */
    template<int size_bits=10, class machine_word=long, class Allocator=void>
    class bits_linear_probe_lru_pool {
        using mw = machine_word;
        static constexpr int sb=size_bits;
        static constexpr int size_of_mw_bytes=sizeof (mw);
        static constexpr int size_of_mw_bits = size_of_mw_bytes<<3;
        static constexpr int items_count = 1<<sb;
        static constexpr mw mm = (mw(1)<<sb)-1;
        static constexpr mw mask_payload = mm;
        static constexpr mw mask_prev = mm << sb;
        static constexpr mw mask_next = mm << (sb+sb);
        static constexpr mw mask_free = mw(1) << (size_of_mw_bits - 1);
        static constexpr mw mask_tombstone = mw(1) << (size_of_mw_bits-2);
        static constexpr mw mask_free_and_tombstone = mask_free | mask_tombstone;
        // data = LSB[...data... | ...prev... | ...next... | config ]MSB
        // data = MSB[ free | tombstone | ...pad... | ...next... | ...prev... | ...data... ]LSB
        struct item_t {
            machine_word key;
            machine_word data;

            inline int value() const { return data & mm; }
            inline int prev() const { return (data>>sb) & mm; }
            inline int next() const { return (data>>(sb<<1)) & mm; }
            inline bool is_tombstone() const { return (data>>(size_of_mw_bits-2)) & mw(1); }
            inline bool is_free() const { return (data>>(size_of_mw_bits-1)) & mw(1); }
            inline void set_value(int value) {
                data = (data & (~mask_payload)) | mw(value & mm);
            }
            inline void set_prev(int value) {
                data = (data & (~mask_prev)) | (mw(value & mm) << sb);
            }
            inline void set_next(int value) {
                data = (data & (~mask_next)) | (mw(value & mm) << (sb<<1));
            }
            inline void set_tombstone_true() { data = data | mask_tombstone; }
            inline void set_tombstone_false() { data = data & (~mask_tombstone); }
            inline void set_free_true() { data = data | mask_free; }
            inline void set_free_false() { data = data & (~mask_free); }
            inline void set_is_free_and_tombstone_true() { data = data | mask_free_and_tombstone; }
            inline void set_is_free_and_tombstone_false() { data = data & (~mask_free_and_tombstone); }
        };

    public:
        using value_type = int;
        using allocator_type = Allocator;
        using rebind_alloc = typename allocator_type::template rebind<item_t>::other;
        struct result_type {
            // the value that was chosen
            int value;
            // the LRU item value that was removed (not a key)
            int removed_value;
            // inserted key was active or free ?
            bool is_active;
        };

        struct iterator_t {
            struct pair {
                machine_word key;
                int value;
            };
            const bits_linear_probe_lru_pool * _c; // container
            int _i; // index

            explicit iterator_t(int i, const bits_linear_probe_lru_pool * c) : _i(i), _c(c) {}
            iterator_t& operator++() {
                if(_i==-1) return *this;
                auto next = _c->_items[_i].next();
                if(next==_c->_mru_list) _i=-1; // reached end
                else _i=next;
                return *this;
            }
            iterator_t& operator--() {
                if(_i==-1) return *this;
                if(_i==_c->_mru_list) _i=-1;
                else _i=_c->_items[_i].prev();
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
            pair operator*() const { return { _c->_items[_i].key, _c->_items[_i].value() }; }
        };

        using const_iterator = iterator_t;
        const_iterator begin() const noexcept { return const_iterator(_mru_list, this); }
        const_iterator end() const noexcept { return const_iterator(-1, this); }

    private:
        item_t * _items;
        int _mru_list, _free_list;
        int _mru_size;
        const int _max_size;
        rebind_alloc _allocator;

        template<class tp> static tp min(const tp & a, const tp & b) { return a<b?a:b; }
        template<class tp> static tp max(const tp & a, const tp & b) { return a>b?a:b; }
        static int compute_max_items(float load_factor) {
            load_factor = min(load_factor, 1.0f);
            int max_items = load_factor * items_count;
            // make sure one free spot is always available
            max_items = min(max_items, items_count-1);
            max_items = max(max_items, 1);
            return max_items;
        }

    public:
        bits_linear_probe_lru_pool(float load_factor=0.5f,
                                   const allocator_type & allocator = allocator_type()) :
                            _items(nullptr), _mru_list(-1), _free_list(-1), _mru_size(0),
                            _max_size(compute_max_items(load_factor)), _allocator(allocator) {
            constexpr bool correcto = (size_of_mw_bytes==4 and (size_bits>=1 and size_bits<=10)) or
                    (size_of_mw_bytes==8 and (size_bits>=1 and size_bits<=20));
            static_assert(correcto, "fail");
            _items = _allocator.allocate(items_count);
            // set linked list
            clear();
        }
        ~bits_linear_probe_lru_pool() {
            _allocator.deallocate(_items, items_count);
        }

        allocator_type get_allocator() { return _allocator; }

        constexpr int capacity() const { return items_count; }
        int size() const { return _mru_size; }
        int maxSize() const { return _max_size; }

    private:
        inline int c2p(machine_word code) const {
            // when size is power of 2, we can get_or_put modulo with
            // bit-wise operation
            return (code & mm);
        }

        int internal_pos_of(machine_word key) const {
            auto start = c2p(key);
            for (int step = 0; step < items_count; ++step) {
                auto pos = c2p(step+start); // modulo
                const auto item = _items[pos];
                if (item.is_tombstone()) continue; // important that this is first
                if (item.is_free()) return -1; // important that this is first
                if (item.key == key) return pos; // found the item with high probability
            }
            return -1;
        }

        void move_attached_node_to_list_head(item_t & node, int pos, int & list) {
            // this item is at the head already, nothing to do
            if(pos == list) return;
            remove_node(node, pos, list);
            insert_detached_node_before(node, pos, list, list);
            list=pos;
        }
        void move_detached_node_to_list_head(item_t & node, int pos, int & list) {
            insert_detached_node_before(node, pos, list, list);
            list=pos;
        }

        void remove_node(const item_t & node, int pos, int & list) {
            // first remove the item
            const auto prev = node.prev();
            const auto next = node.next();
            _items[prev].set_next(next);
            _items[next].set_prev(prev);
            //
            if(pos == list) list = (next!=pos ? next : -1);
        }

        void insert_detached_node_before(item_t & node, int pos, int before, int & list) {
            item_t & before_node = _items[before];
            if(before >= 0) {
                auto before_node_prev = before_node.prev();
                node.set_next(before);
                node.set_prev(before_node_prev);
                //
                _items[before_node_prev].set_next(pos);
                before_node.set_prev(pos);
            }
            else { // first item
                node.set_next(pos);
                node.set_prev(pos);
                list=pos;
            }
        }

        int adjust_load_factor() {
            int delta = _mru_size - _max_size;
            if(delta<=0) return -1;
            // now, let's move nodes from active lru to free list
            for(; delta and _mru_list != -1; --delta) {
                const auto pos = _items[_mru_list].prev(); // tail is LRU
                auto & node = _items[pos];
                internal_remove_key_node(node, pos);
            }
        }

        /**
         * remove just one excess item and return the int value, that
         * became free
         */
        int adjust_load_factor_remove_one() {
            int delta = _mru_size - _max_size;
            if(delta<=0) return -1;
            const auto pos = _items[_mru_list].prev(); // tail is LRU
            auto & node = _items[pos];
            int removed_value = node.value();
            internal_remove_key_node(node, pos);
            return removed_value;
        }

    public:
        bool has(machine_word key) const { return internal_pos_of(key) + 1; }
        int get(machine_word key) {
            const auto pos = internal_pos_of(key);
            // report -1 if key not found
            if(pos==-1) return -1;
            auto & item = _items[pos];
            // update LRU list
            move_attached_node_to_list_head(item, pos, _mru_list);
            // return its value
            return item.value();
        }

        result_type get_or_put(machine_word key) {
            int removed_value = adjust_load_factor_remove_one();
            auto start = c2p(key);
            int first_free_pos=-1;
            for (int step = 0; step < items_count; ++step) {
                auto pos = c2p(step+start); // modulo
                item_t & item = _items[pos];
                if (item.is_tombstone()) {
                    // skip over tombstone but remember
                    if(first_free_pos==-1) first_free_pos = pos;
                    continue; // important that this is first
                }
                if (item.is_free()) {
                    if(first_free_pos==-1) first_free_pos = pos;
                    break;
                } // important that this is first
                if (item.key == key) { // found the item with high probability
                    move_attached_node_to_list_head(item, pos, _mru_list);
                    return {item.value(), removed_value, true};
                }
            }
            // if we got here, key was not found, let's put it one of the free
            // location we found.

            // error, key not found and no free place to allocate
            if(first_free_pos==-1) return { -1, -1, false };
            item_t & item = _items[first_free_pos];
            item.set_is_free_and_tombstone_false();
            item.key=key;
            remove_node(item, first_free_pos, _free_list);
            move_detached_node_to_list_head(item, first_free_pos, _mru_list);
            ++_mru_size;
            return { item.value(), removed_value, false };
        }

    private:
        void internal_remove_key_node(item_t & node, int start) {
            // we assume node is active
            remove_node(node, start, _mru_list);
            node.set_is_free_and_tombstone_true();
            move_detached_node_to_list_head(node, start, _free_list);
            --_mru_size;
        }

    public:
        int remove(machine_word key) {
            auto start = internal_pos_of(key);
            if(start==-1) return -1;
            auto & removed_item = _items[start];
            int removed_item_value = removed_item.value();
            internal_remove_key_node(removed_item, start);
            return removed_item_value;
        }

        void clear() {
            for (int ix = 0; ix < items_count; ++ix) {
                item_t & item = _items[ix];
                item.key=0;
                item.set_value(ix);
                item.set_prev(ix-1);
                item.set_next(ix+1);
                item.set_free_true();
                item.set_tombstone_false();
            }
            // a node is head/tail if it's prev/next is itself
            _mru_size=0;
            _mru_list=-1;
            _free_list=0;
            _items[items_count-1].set_next(_free_list);
            _items[_free_list].set_prev(items_count-1);
        }

        void print(char order=LRU_PRINT_ORDER_MRU, int how_many=-1) const {
#ifdef LRU_CACHE_ALLOW_PRINT
            const bool order_seq = order==LRU_PRINT_SEQ;
            const bool order_mru = order==LRU_PRINT_ORDER_MRU;
            const bool order_free = order==LRU_PRINT_ORDER_FREE_LIST;
            int start = order_seq ? 0 : order_mru ? _mru_list : _free_list;
            int stop = order_seq ? items_count : order_mru ? _mru_list : _free_list;
            const char * str_order = order_seq ? "SEQUENCE" : order_mru ? "MRU" : "FREE";
            std::cout << "\n====== Printing in " << str_order << " order \n"
                      << "- LRU head is " << _mru_list << ", free head is "
                      << _free_list << "\n";
            std::cout << "- MAX SIZE is " << _max_size << ", LOAD FACTOR is " << float(maxSize())/capacity()
                      << "\n";
            std::cout << "- LRU size is " << _mru_size << ", FREE size is "
                      << (capacity() - _mru_size) << ", CAPACITY is " << capacity() << '\n';
            std::cout << "- Printing " << (how_many==-1 ? "All" : std::to_string(how_many)) << " Items \n";
            std::cout << "[\n";
            if(start==-1) return;
            do {
                if(how_many--==0) break;
                const item_t item = _items[start];
                const char * head_str = start == _mru_list ? "* " : start == _free_list ? "$ " : "";
                std::cout << head_str << start << " = ( K: " << item.key << ", V: " << item.value()
                          << ", free: " << item.is_free() << ", tomb: " << item.is_tombstone()
                          << ", <-: " << item.prev() << ", ->: " << item.next()
                          << " ),\n";
                start = order_seq ? (start + 1) : item.next();
            } while(start != stop);
            std::cout << "]\n";
#endif
        }

    };

}