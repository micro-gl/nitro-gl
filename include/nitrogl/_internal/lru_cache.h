#pragma once

namespace nitrogl {

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
            machine_word hash_code;
            machine_word data;

            inline int payload() { return data & mm; }
            inline int prev() { return (data>>sb) & mm; }
            inline int next() { return (data>>(sb<<1)) & mm; }
            inline int config() { return (data>>(size_of_mw_bits-1)) & mw(1); }
            inline bool is_free() {
                return !config();
            }
            inline void set_payload(int value) {
                data = (data & (~mask_payload)) | (value & mm);
            }
            inline void set_prev(int value) {
                data = (data & (~mask_prev)) | (value << sb);
            }
            inline void set_next(int value) {
                data = (data & (~mask_next)) | (value << (sb<<1));
            }
            inline void set_config(int value) {
                data = (data & (~mask_config)) | (value << (size_of_mw_bits-1));
            }
            inline void set_is_free(bool is_free) {
                set_config(is_free ? 0 : 1);
            }

        };

        item_t _items[items_count];
        int _head, _tail;


    public:
        lru_cache() : _items{}, _head(-1), _tail(-1) {
            // set linked list
            for (int ix = 0; ix < items_count; ++ix) {
                auto item = _items[ix];
                item.hash_code=0;
                item.set_payload(ix);
                item.set_prev(ix-1);
                item.set_next(ix+1);
            }
            _head=0;
            _tail=items_count-1;
            _items[_tail].set_next(-1);
        }

        constexpr int size() const { return items_count; }

        inline int c2p(machine_word code) const {
            // when size is power of 2, we can get modulo with
            // bit-wise operation
            return (code & mm);
        }

        inline int distance_to_home_of(machine_word code, int current_home) {
            // this is to avoid branching due to current home wraping around
            return c2p((current_home - c2p(code)) + items_count);
        }

        int internal_pos_of(machine_word hash_code) const {
            auto start = c2p(hash_code);
            for (int step = 0; step < items_count; ++step) {
                auto pos = c2p(step+start); // modulo
                const auto item = _items[pos];
                if (item.is_free()) return -1; // important that this is first
                if (item.hash_code==hash_code) return pos; // found the item with high probability
                if (distance_to_home_of(item.hash_code, pos)<step) {
                    // early stop detection, we found a non-free, that was closer to home,
                    return -1;
                }
            }
        }

        bool has(machine_word code) {
            return internal_pos_of(code)+1;
        }

        struct query_type {
            int payload;
            bool found;
        };

        void move_to_head(int pos) {
            auto item = _items[pos];

            const auto prev = item.prev();
            // this item is at the head already, nothing to do
            if(prev==-1) return;

            // first remove the item
            const auto next = item.next();
            _items[prev].set_next(next);
            if(next!=-1) {
                _items[next].set_prev(prev);
            } else _tail=prev;

            // now put it in the head
            _items[_head].set_prev(pos);
            _items[pos].set_next(_head);
            _items[pos].set_prev(-1);
            _head=pos;
        }

        query_type get(machine_word code) {
            const auto pos = internal_pos_of(code);
            if(pos==-1) return { 0, false };
            int payload = _items[pos].payload();
            // lru-cache update
            move_to_head(pos);
            return { payload, true };
        }

        void put(machine_word key, int value) {
            auto start = c2p(key);
            item_t next_displaced_item {};
            // first iterations to find a spot
            for (int step = 0; step < items_count; ++step) {
                const auto pos = c2p(start + step); // modulo
                auto & item = _items[pos];
                if (item.is_free()) { // free item, let's conquer
                    // put the item
                    item.hash_code = key;
                    item.set_payload(value);
                    item.set_is_free(false);
                    move_to_head(pos);
                    return;
                }
                if (item.hash_code==key) { // found, let's update value
                    item.set_payload(value);
                    move_to_head(pos);
                    return;
                }
                if (distance_to_home_of(item.hash_code, pos)<step) { // let's robin hood
                    // swap
                    next_displaced_item = item;
                    item.hash_code = key;
                    item.set_payload(value);
                    // move to head, special case ignoring neighborhood of previous member
                    // now put it in the head
                    _items[_head].set_prev(pos);
                    _items[pos].set_next(_head);
                    _items[pos].set_prev(-1);
                    _head=pos;

                    // displace next iterations
                    start = pos+1;
                    break;
                }
            }

            // now displacements, this is not in the above loop because lru cache
            // requires some mods.
            bool has_pending_displace=true;
            while(has_pending_displace) {
                has_pending_displace=false;
                for (int step = 0; step < items_count; ++step) {
                    const auto pos = c2p(start + step); // modulo
                    auto & item = _items[pos];
                    if (item.is_free()) { // free item, let's conquer
                        // put the item
                        item = next_displaced_item;
                        return;
                    }
                    if (distance_to_home_of(item.hash_code, pos)<step) { // let's robin hood
                        // swap
                        const auto temp = next_displaced_item;
                        next_displaced_item = item;
                        item = temp;
                        // update lru linked list, only change its siblings
                        const auto item_prev = item.prev();
                        const auto item_next = item.next();
                        if(item_prev!=-1) _items[item_prev].set_next(pos);
                        if(item_next!=-1) _items[item_next].set_prev(pos);
                        // displace start for next iterations
                        start = pos+1;
                        has_pending_displace=true;
                        break;
                    }
                }
            }

        }

        void remove(machine_word key) {
            const auto start = internal_pos_of(key);
            if(start==-1) return;

            _items[start].set_is_free(true);
            ++start;
            for (int step = 0; step < items_count; ++step) {
                const auto pos = c2p(start + step); // modulo

                auto & item = _items[pos];
                // we are done when the item in question is free or it's distance
                // from home is 0
                if(item.is_free()) return;
                if(distance_to_home_of(item.hash_code, pos)==0) return;
                // other-wise, we need to move it left because it's left sibling is empty
                const auto pos_predecessor = c2p(start + step - 1); // modulo
                _items[pos_predecessor] = item;
                // now update the lru linked list of it's siblings to it's new pos
                const auto item_prev = item.prev();
                const auto item_next = item.next();
                if(item_prev!=-1) _items[item_prev].set_next(pos_predecessor);
                if(item_next!=-1) _items[item_next].set_prev(pos_predecessor);
            }
        }

    };
}