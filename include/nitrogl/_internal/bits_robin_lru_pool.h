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
     * 2. upto 21 bits per value for 64 bits keys
     * This uses a hash table with robin hood probing and in-place linked-list,
     * and is very conservative with memory, which allows CPU caches to load many entries at once.
     *
     * NOTES:
     * - Keys are assumed to be unique (if you are using hash function as a source for keys use a good one)
     * - for 32 bits keys, each entry is 64 bit.
     * - for 64 bits keys, each entry is 128 bit.
     * - The size of the cache is a power of 2 of the bits for value, which gives some optimizations.
     * - perfect for small caches: up to 1024 entries for 32 bit keys and 2,097,152 for 64 bit keys.
     * - perfect for storing integer indices.
     *
     * @tparam size_bits the integer bits size
     * @tparam machine_word the machine word type = short, int or long
     */
    template<int size_bits=10, class machine_word=long, class Allocator=void>
    class bits_robin_lru_pool {
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
        // data = LSB[...data... | ...prev... | ...next... | free ]MSB
        // data = MSB[ free | ...pad... | ...next... | ...prev... | ...data... ]LSB
        struct item_t {
            machine_word key;
            machine_word data;

            inline int value() const { return data & mm; }
            inline int prev() const { return (data>>sb) & mm; }
            inline int next() const { return (data>>(sb<<1)) & mm; }
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
            inline void set_is_free_true() { data = data | mask_free; }
            inline void set_is_free_false() { data = data & (~mask_free); }
        };

    public:
        using value_type = int;
        using allocator_type = Allocator;
        using rebind_alloc = typename allocator_type::template rebind<item_t>::other;
        struct result_type {
            // the value that was chosen
            int value;
            // the LRU item value that became free (not a key, but a value)
            int removed_value;
            // inserted key was active or free ?
            bool is_active;
        };

        struct iterator_t {
            struct pair {
                machine_word key;
                int value;
            };
            const bits_robin_lru_pool * _c; // container
            int _i; // index

            explicit iterator_t(int i, const bits_robin_lru_pool * c) : _i(i), _c(c) {}
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

        bits_robin_lru_pool(float load_factor=0.5f, const allocator_type & allocator = allocator_type()) :
                            _items(nullptr), _mru_list(-1), _free_list(-1), _mru_size(0),
                            _max_size(compute_max_items(load_factor)), _allocator(allocator) {
            constexpr bool correcto = (size_of_mw_bytes==4 and (size_bits>=1 and size_bits<=10)) or
                    (size_of_mw_bytes==8 and (size_bits>=1 and size_bits<=21));
            static_assert(correcto, "fail");
            _items = _allocator.allocate(items_count);
            // set linked list
            clear();
        }
        ~bits_robin_lru_pool() {
            _allocator.deallocate(_items, items_count);
            _items= nullptr;
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

        inline int distance_to_home_of(machine_word code, int current_home) const {
            // this is to avoid branching due to current home wrapping around
            return c2p((current_home - c2p(code)) + items_count);
        }

        int internal_pos_of(machine_word key) const {
            auto start = c2p(key);
            for (int step = 0; step < items_count; ++step) {
                auto pos = c2p(step+start); // modulo
                const auto & item = _items[pos];
                if (item.is_free()) return -1; // important that this is first
                if (item.key == key) return pos; // found the item with high probability
                if (distance_to_home_of(item.key, pos) < step) {
                    // early stop detection, we found a non-free, that was closer to home,
                    return -1;
                }
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

        void swap_detached_items(item_t & a, item_t & b) {
            // note: items have to be detached
            item_t c = a;
            a=b; b=c;
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
        void adjust_load_factor() {
            int delta = _mru_size - _max_size;
            if(delta<=0) return;
            // now, let's move nodes from active lru to free list
            for(; delta and _mru_list != -1; --delta) {
                const auto pos = _items[_mru_list].prev(); // tail is LRU
                auto & node = _items[pos];
                internal_remove_key_node(node, pos);
            }
        }

    public:
        bool has(machine_word key) const { return internal_pos_of(key) + 1; }
        /**
         * query the value of a key without affecting the LRU list.
         * 1. If key is present, return it's value
         * 2. otherwise, return -1
         * @param key
         * @return
         */
        int value_of(machine_word key) {
            const auto pos = internal_pos_of(key);
            return pos>=0 ? _items[pos].value() : -1;
        }

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

        /**
         * this will get_or_put the value of the key if exists or insert a new one and
         * get_or_put it's free value from the pool. Also, will update the LRU list.
         * @param key
         * @return
         */
        result_type get_or_put(machine_word key) {
            int removed_value = adjust_load_factor_remove_one();
            auto start = c2p(key);
            item_t next_displaced_item {};
            int base_dist_of_displaced=0;
            int first_pos_to_displace = -1;
            // first iterations to find a spot
            for (int step = 0; step < items_count; ++step) {
                const auto pos = c2p(start + step); // modulo
                item_t & item = _items[pos];
                if (item.is_free()) {
                    // didn't find the key, let's take a free one instead.
                    item.key = key;
                    item.set_is_free_false();
                    remove_node(item, pos, _free_list);
                    move_detached_node_to_list_head(item, pos, _mru_list);
                    ++_mru_size;
                    return { item.value(), removed_value, false };
                }
                if (item.key == key) { // found the key, let's return it
                    move_attached_node_to_list_head(item, pos, _mru_list);
                    return { item.value(), removed_value, true };
                }
                base_dist_of_displaced = distance_to_home_of(item.key, pos);
                if (base_dist_of_displaced < step) {
                    // early stop detection, the key is not present if we hit this condition.
                    // lets robin hood steal from this place and use a new free item.
                    first_pos_to_displace = pos;
                    // swap
                    next_displaced_item = item;
                    // no need to set value yet, only at the end.
                    item.key = key;
                    // we postpone the value to the first free item found during
                    // the upcoming forward pass
                    // move to head
                    move_attached_node_to_list_head(item, pos, _mru_list);
                    // next displaced item start pos
                    start = pos;
                    ++_mru_size;
                    break;
                }
            }
            // now displacements, this is not in the above loop because lru cache
            // requires some mods. NONE of the displaced items can be heads.
            bool has_pending_displace=true;
            while(has_pending_displace) {
                has_pending_displace=false;
                for (int step = 1; step < items_count; ++step) {
                    const auto pos = c2p(start + step); // modulo
                    auto & item = _items[pos];
                    if (item.is_free()) { // free item, let's conquer
                        remove_node(item, pos, _free_list);
                        // we take the value of this free node and put it in the first
                        // node that initiated the first displace
                        const int value = item.value();
                        _items[first_pos_to_displace].set_value(value);
                        // override with previously displaced item
                        item = next_displaced_item;
                        insert_detached_node_before(item, pos, item.next(), _mru_list);
                        // finished back-shift, let's return;
                        return { value, removed_value, false };
                    }
                    int item_dist = distance_to_home_of(item.key, pos);
                    if (item_dist < base_dist_of_displaced + step) { // let's robin hood
                        // before all, current displaced item might have wanted to move
                        // to one of its siblings(prev/next), so make a copy and update them.
                        const auto temp = next_displaced_item;
                        next_displaced_item = item;
                        if(temp.next() == pos) next_displaced_item.set_prev(pos);
                        if(temp.prev() == pos) next_displaced_item.set_next(pos);
                        // now, detach current node
                        remove_node(item, pos, _mru_list);
                        // assign displaced
                        item = temp;
                        // update lru linked list, only change its siblings
                        insert_detached_node_before(item, pos, item.next(), _mru_list);
                        // prepare for next iteration
                        base_dist_of_displaced = item_dist;
                        start = pos;
                        has_pending_displace=true;
                        break;
                    }
                }
            }
            return { -1, -1, false };
        }

    private:
        void internal_remove_key_node(item_t & node, int start) {
            remove_node(node, start, _mru_list);
            node.set_is_free_true();
            move_detached_node_to_list_head(node, start, _free_list);
            --_mru_size;
            ++start;
            // begin back shifting procedure
            for (int step = 0; step < items_count; ++step) {
                auto pos = c2p(start + step); // modulo
                auto & item = _items[pos];
                // we are done when the item in question is free or it's distance
                // from home is 0
                if(item.is_free()) return;
                if(distance_to_home_of(item.key, pos) == 0) return;
                // other-wise, we need to move it left because it's left sibling is empty
                const auto pos_predecessor = c2p(start + step - 1); // modulo
                auto & predecessor = _items[pos_predecessor];
                bool is_pos_head = pos == _mru_list;
                remove_node(item, pos, _mru_list);
                remove_node(predecessor, pos_predecessor, _free_list);
                int insert_pred_before = is_pos_head ? _mru_list : item.next();
                swap_detached_items(item, predecessor);
                // order of active items is important so we recorded
                insert_detached_node_before(predecessor, pos_predecessor,
                                            insert_pred_before, _mru_list);
                if(is_pos_head) _mru_list=pos_predecessor;
                // the order of free items is not important
                move_detached_node_to_list_head(item, pos, _free_list);
            }
        }

    public:
        /**
         * removes active item and returns its value, which serves as an index for users
         * @param key
         * @return
         */
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
                auto & item = _items[ix];
                item.key=0;
                item.set_value(ix);
                item.set_prev(ix-1);
                item.set_next(ix+1);
                item.set_is_free_true();
            }
            // a node is head/tail if it's prev/next is itself
            _mru_size=0;
            _mru_list=-1;
            _free_list=0;
            _items[items_count-1].set_next(_free_list);
            _items[_free_list].set_prev(items_count-1);
        }

    void print(char order=1, int how_many=-1) const {
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
                const auto item = _items[start];
                const char * head_str = start == _mru_list ? "* " : start == _free_list ? "$ " : "";
                std::cout << head_str << start << " = ( K: " << item.key << ", V: "
                          << item.value() << ", free: " << item.is_free() << ", <-: "
                          << item.prev() << ", ->: " << item.next() << " ),\n";
                start = order_seq ? (start + 1) : item.next();
            } while(start != stop);
            std::cout << "]\n";
#endif
        }

    };

}