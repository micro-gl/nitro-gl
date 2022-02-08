#pragma once

namespace nitrogl {

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
    template<int size_bits=10, class machine_word=long>
    class lru_cache {
        using mw = machine_word;
        static constexpr int sb=size_bits;
        static constexpr int size_of_mw_bytes=sizeof (mw);
        static constexpr int size_of_mw_bits = size_of_mw_bytes<<3;
        static constexpr int items_count = 1<<sb;
        static constexpr mw mm = (mw(1)<<sb)-1;
        static constexpr mw mask_payload = mm;
        static constexpr mw mask_prev = mm << sb;
        static constexpr mw mask_next = mm << (sb+sb);
        static constexpr mw mask_config = mw(1) << (size_of_mw_bits-1);
        // data = LSB[...data... | ...prev... | ...next... | config ]MSB
        // data = MSB[ config | ...pad... | ...next... | ...prev... | ...data... ]LSB
        struct item_t {
            machine_word key;
            machine_word data;

            inline int value() const { return data & mm; }
            inline int prev() const { return (data>>sb) & mm; }
            inline int next() const { return (data>>(sb<<1)) & mm; }
            inline int config() const { return (data>>(size_of_mw_bits-1)) & mw(1); }
            inline bool is_free() const { return !config(); }
            inline void set_value(int value) {
                data = (data & (~mask_payload)) | (value & mm);
            }
            inline void set_prev(int value) {
                data = (data & (~mask_prev)) | ((value & mm) << sb);
            }
            inline void set_next(int value) {
                data = (data & (~mask_next)) | ((value & mm) << (sb<<1));
            }
            inline void set_config(int value) {
                data = (data & (~mask_config)) | ((value & 1) << (size_of_mw_bits-1));
            }
            inline void set_is_free(bool is_free) {
                set_config(is_free ? 0 : 1);
            }

        };

        item_t _items[items_count];
        int _lru_list, _free_list;

    public:
        using value_type = int;

        lru_cache() : _items{}, _lru_list(-1), _free_list(-1) {
            // set linked list
            for (int ix = 0; ix < items_count; ++ix) {
                auto & item = _items[ix];
                item.key=0;
                item.set_value(ix);
                item.set_prev(ix-1);
                item.set_next(ix+1);
                item.set_is_free(true);
            }
            // a node is head/tail if it's prev/next is itself
            _lru_list=-1;
            _free_list=0;
            _items[items_count-1].set_next(_free_list);
            _items[_free_list].set_prev(items_count-1);
        }

    private:
        inline int c2p(machine_word code) const {
            // when size is power of 2, we can get modulo with
            // bit-wise operation
            return (code & mm);
        }

        inline int distance_to_home_of(machine_word code, int current_home) const {
            // this is to avoid branching due to current home wraping around
            return c2p((current_home - c2p(code)) + items_count);
        }

        int internal_pos_of(machine_word key) const {
            auto start = c2p(key);
            for (int step = 0; step < items_count; ++step) {
                auto pos = c2p(step+start); // modulo
                const auto item = _items[pos];
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
//            auto & item = _items[pos];
            // first remove the item
            const auto prev = node.prev();
            const auto next = node.next();
            _items[prev].set_next(next);
            _items[next].set_prev(prev);
            //
            if(pos == list) list = (next!=pos ? next : -1);
        }

        void insert_detached_node_before(item_t & node, int pos, int before, int & list) {
//            item_t & node = _items[pos];
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

    public:
        constexpr int size() const { return items_count; }

        bool has(machine_word key) const {
            return internal_pos_of(key) + 1;
        }

        struct query_type {
            int payload;
            bool found;
        };

        query_type get(machine_word key) {
            const auto pos = internal_pos_of(key);
            if(pos==-1) return { 0, false };
            auto & node = _items[pos];
            // lru-cache update
            move_to_list_head(node, pos, _lru_list);
            return { node.value(), true };
        }

        void put(machine_word key, int value) {
            auto start = c2p(key);
            item_t next_displaced_item {};
            int base_dist=0;
            // first iterations to find a spot
            for (int step = 0; step < items_count; ++step) {
                const auto pos = c2p(start + step); // modulo
                auto & item = _items[pos];
                if (item.is_free()) { // free item, let's conquer
                    // put the item
                    item.key = key;
                    item.set_value(value);
                    item.set_is_free(false);
                    remove_node(item, pos, _free_list);
                    move_detached_node_to_list_head(item, pos, _lru_list);
                    return;
                }
                if (item.key == key) { // found, let's update value
                    item.set_value(value);
                    move_attached_node_to_list_head(item, pos, _lru_list);
                    return;
                }
                base_dist = distance_to_home_of(item.key, pos);
                if (base_dist < step) { // let's robin hood
                    // swap
                    next_displaced_item = item;
                    item.key = key;
                    item.set_value(value);
                    // move to head
//                    move_to_list_head(pos, false);
                    move_attached_node_to_list_head(item, pos, _lru_list);
                     // displace next iterations
                    start = pos;
                    break;
                }
            }
//            print();
//            return;
            // now displacements, this is not in the above loop because lru cache
            // requires some mods. NONE of the displaced items can be heads.
            bool has_pending_displace=true;
            while(has_pending_displace) {
                has_pending_displace=false;
                for (int step = 1; step < items_count; ++step) {
                    const auto pos = c2p(start + step); // modulo
                    auto & item = _items[pos];
                    if (item.is_free()) { // free item, let's conquer
                        // now item was copied, he has linked-list info but his
                        // pred/succ do not point to him, so let's fix it
                        remove_node(item, pos, _free_list);
                        item = next_displaced_item;
                        insert_detached_node_before(item, pos, item.next(), _lru_list);
                        return;
                    }
                    int item_dist = distance_to_home_of(item.key, pos);
                    if (item_dist < base_dist+step) { // let's robin hood
                        // before all, current displaced item might have wanted to move
                        // to one of its siblings(prev/next), so make a copy and update them.
                        const auto temp = next_displaced_item;
                        next_displaced_item = item;
                        if(temp.next() == pos) next_displaced_item.set_prev(pos);
                        if(temp.prev() == pos) next_displaced_item.set_next(pos);

                        // now, detach current node
                        remove_node(item, pos, _lru_list);
                        // assign displaced
                        item = temp;
                        // update lru linked list, only change its siblings
                        insert_detached_node_before(item, pos, item.next(), _lru_list);
//                        print();
                        // prepare for next iteration
                        base_dist = item_dist;
                        start = pos;
                        has_pending_displace=true;
                        break;
                    }
                }
//                print();
            }

        }

        void remove(machine_word key) {
            auto start = internal_pos_of(key);
            if(start==-1) return;

            auto & removed_item = _items[start];
            remove_node(removed_item, start, _lru_list);
            removed_item.set_is_free(true);
            move_detached_node_to_list_head(removed_item, start, _free_list);
            ++start;
//            print();
//            return;
            // begin back shifting procedure
            for (int step = 0; step < items_count; ++step) {
                const auto pos = c2p(start + step); // modulo

                auto & item = _items[pos];
                // we are done when the item in question is free or it's distance
                // from home is 0
                if(item.is_free()) return;
                if(distance_to_home_of(item.key, pos) == 0) return;
                // other-wise, we need to move it left because it's left sibling is empty
                const auto pos_predecessor = c2p(start + step - 1); // modulo
                bool is_pos_head = pos == _lru_list;
                remove_node(item, pos, _lru_list);
                auto & predecessor = _items[pos_predecessor];
                remove_node(predecessor, pos_predecessor, _free_list);
                predecessor = item;
                int insert_pred_before = is_pos_head ? _lru_list : predecessor.next();
                insert_detached_node_before(predecessor, pos_predecessor, insert_pred_before, _lru_list);
                if(is_pos_head) _lru_list=pos_predecessor;
                item.set_is_free(true);
                move_detached_node_to_list_head(item, pos, _free_list);
            }
        }

#define LRU_PRINT_SEQ 0
#define LRU_PRINT_ORDER_MRU 1
#define LRU_PRINT_ORDER_FREE_LIST 2

    void print(char order=1) const {
            const bool order_seq = order==LRU_PRINT_SEQ;
            const bool order_mru = order==LRU_PRINT_ORDER_MRU;
            const bool order_free = order==LRU_PRINT_ORDER_FREE_LIST;
            int start = order_seq ? 0 : order_mru ? _lru_list : _free_list;
            int stop = order_seq ? items_count : order_mru ? _lru_list : _free_list;
            const char * str_order = order_seq ? "SEQUENCE" : order_mru ? "MRU" : "FREE";
            std::cout << "\n====== Printing in " << str_order << " order \n"
                      << "- mru head is " << _lru_list << ", free head is "
                      << _free_list << "\n[\n";
            if(start==-1) return;
            int counter=items_count;
            do {
                const auto item = _items[start];
                const char * head_str = start == _lru_list ? "* " : start == _free_list ? "$ " : "";
                std::cout << head_str << start << " = ( K: " << item.key << ", V: "
                          << item.value() << ", free: " << item.is_free() << ", <-: "
                          << item.prev() << ", ->: " << item.next() << " ),\n";
                start = order_seq ? (start + 1) : item.next();
                ++counter;
            } while(start != stop && counter);
            std::cout << "]\n";
        }

    };

}