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
        int _head, _free_list;

    public:
        using value_type = int;

        lru_cache() : _items{}, _head(-1), _free_list(-1) {
            // set linked list
//            for (int ix = 0; ix < items_count; ++ix) {
//                auto & item = _items[ix];
//                item.key=0;
//                item.set_value(ix);
//                item.set_prev(ix-1);
//                item.set_next(ix+1);
//                item.set_is_free(true);
//            }
            // a node is head/tail if it's prev/next is itself
            _head=-1;
//            _free_list=0;
//            _items[items_count-1].set_is_free(true);
//            _items[items_count-1].set_next(_free_list);
//            _items[_free_list].set_prev(items_count-1);
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

        void move_to_head(int pos, bool is_free) {
            // this item is at the head already, nothing to do
            if(pos==_head) return;
            if(!is_free) remove_node(pos);
            insert_detached_node_before(pos, _head);
            _head=pos;
            return;

            auto & item = _items[pos];

            // first remove the node
            {
                const auto prev = item.prev();
                const auto next = item.next();
                _items[prev].set_next(next);
                _items[next].set_prev(prev);
            }
            // now put it in the head
            {
                auto & head_item = _items[_head];
                auto & tail_item = _items[head_item.prev()];
                item.set_next(_head);
                item.set_prev(head_item.prev());
                head_item.set_prev(pos);
                tail_item.set_next(pos);
            }
            // tag
            _head=pos;
        }

        void remove_node(int pos) {
            auto & item = _items[pos];
            // first remove the item
            const auto prev = item.prev();
            const auto next = item.next();
            _items[prev].set_next(next);
            _items[next].set_prev(prev);
            //
            if(pos==_head) _head = next!=pos ? next : -1;
        }

        void insert_detached_node_before(int pos, int before) {
            item_t & node = _items[pos];
            item_t & before_node = _items[before];
            if(before >= 0) {
                node.set_next(before);
                node.set_prev(before_node.prev());
                //
                _items[before_node.prev()].set_next(pos);
                before_node.set_prev(pos);
            }
            else { // first item
                node.set_next(pos);
                node.set_prev(pos);
                _head=pos;
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
            int payload = _items[pos].value();
            // lru-cache update
            move_to_head(pos, false);
            return { payload, true };
        }

        void put(machine_word key, int value) {
            auto start = c2p(key);
            item_t next_displaced_item {};
            int head_pos=0;
            // first iterations to find a spot
            for (int step = 0; step < items_count; ++step) {
                const auto pos = c2p(start + step); // modulo
                auto & item = _items[pos];
                if (item.is_free()) { // free item, let's conquer
                    // put the item
                    item.key = key;
                    item.set_value(value);
                    item.set_is_free(false);
                    move_to_head(pos, true);
                    return;
                }
                if (item.key == key) { // found, let's update value
                    item.set_value(value);
                    move_to_head(pos, false);
                    return;
                }
                if (distance_to_home_of(item.key, pos) < step) { // let's robin hood
                    // swap
                    next_displaced_item = item;
                    item.key = key;
                    item.set_value(value);
                    // move to head, special case ignoring neighborhood of previous member.
                    // displaced item takes his linked list info with him. So we just ignore and
                    // new item at head
                    // now put it in the head
                    // connect to head
                    move_to_head(pos, false);
                    // displace next iterations
                    start = pos;
                    head_pos=pos;
                    break;
                }
            }
            print();
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
                        item = next_displaced_item;
                        // now item was copied, he has linked-list info but his
                        // pred/succ do not point to him, so let's fix it
                        insert_detached_node_before(pos, item.next());
                        return;
                    }
                    if (distance_to_home_of(item.key, pos) < step) { // let's robin hood
                        // swap
                        // before all, update siblings, because current pos might be the sibling
                        const auto temp = next_displaced_item;
                        next_displaced_item = item;
                        if(temp.next() == pos) next_displaced_item.set_prev(pos);
                        if(temp.prev() == pos) next_displaced_item.set_next(pos);

                        // now, detach current node
                        remove_node(pos);
                        // assign displaced
                        item = temp;
                        // update lru linked list, only change its siblings
                        insert_detached_node_before(pos, item.next());
//                        print();
                        start = pos;
                        has_pending_displace=true;
                        break;
                    }
                }
            }

        }

        void remove(machine_word key) {
            auto start = internal_pos_of(key);
            if(start==-1) return;

            auto & removed_item = _items[start];
            remove_node(start);
            removed_item.set_is_free(true);
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
                bool is_pos_head = pos==_head;
                remove_node(pos);
                _items[pos_predecessor] = item;
                insert_detached_node_before(pos_predecessor, item.next());
                if(is_pos_head) _head=pos_predecessor;
                item.set_is_free(true);
            }
        }

        void print() const {
            int curr = _head;
            std::cout << "\n====== Printing in MRU \n[\n";
            if(curr==-1) { std::cout << "- empty !!!\n"; return; }
            int counter=items_count;
            do {
                const auto item = _items[curr];
                std::cout << curr << " = ( K: " << item.key << ", V: "
                          << item.value() << ", free: " << item.is_free() << ", <-: "
                          << item.prev() << ", ->: " << item.next() << " ),\n";
                curr = item.next();
                --counter;
            } while(curr!=_head && counter);
            std::cout << "]\n";
        }

    };

}