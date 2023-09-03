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
    namespace detail {
        template<int T>
        struct identity { constexpr static int N = T; };
    }

    /**
     * Iterative murmur2A Hash function, both for 32 and 64 bit machines.
     * - Start with a seed at constructor
     * - Feed words iteratively with the next() method
     * - Finish with end() method to get the result
     * - This takes into account the length of the sequence, so NULL keys
     * @tparam machine_word should be unsigned integer or long (32 bit or 64 bit wide word)
     */
    template<class machine_word>
    struct iterative_murmur {
    private:
        template<class T1, class T2>
        struct is_same { constexpr static bool value = false; };
        template<class T> struct is_same<T, T> { constexpr static bool value = true; };

        using mw = machine_word;
        static constexpr char _bits_ = sizeof(mw)<<3;
        static constexpr int r = _bits_==32 ? 24 : 47;
        static constexpr mw m = _bits_==32 ? 0x5bd1e995 : 0xc6a4a7935bd1e995;
        machine_word _state;
        machine_word _len;
#define mmix(h,k) { (k) *= m; (k) ^= (k) >> r; (k) *= m; (h) *= m; (h) ^= (k); }

    public:
        explicit iterative_murmur() : _state(0), _len(0) {
            constexpr bool _32_or_64 = sizeof(mw)==4 or sizeof(mw)==8;
            constexpr bool _is_unsigned = is_same<mw, unsigned long long>::value or
                            is_same<mw, unsigned long>::value or
                            is_same<mw, unsigned int>::value;
            static_assert(_32_or_64, "machine-word must be 32 or 64 bit");
            static_assert(_is_unsigned, "machine-word must be unsigned");
        }

        template<class VV> iterative_murmur begin_cast(VV seed) {
            return begin(reinterpret_cast<machine_word>(seed));
        }
        iterative_murmur begin(machine_word seed) {
            _state=seed; _len=0;
            return *this;
        }

        template<class VV> iterative_murmur next_cast(VV payload) {
            return next(reinterpret_cast<machine_word>(payload));
        }
        iterative_murmur next(machine_word payload) {
            ++_len;
            _next(payload, detail::identity<_bits_>());
            return *this;
        }

        machine_word end() {
            return _end(detail::identity<_bits_>());
        }

    private:
//        template<char bits>
//        inline void _next(machine_word payload) {}

//        template<char bits>
//        inline machine_word _end() {}

        inline void _next(machine_word payload, detail::identity<32>) {
            mmix(_state, payload)
        }

        inline void _next(machine_word payload, detail::identity<64>) {
            mmix(_state, payload)
        }

        inline machine_word _end(detail::identity<32>) {
            mw len = _len;
            mmix(_state, len)
            _state ^= _state >> 13;
            _state *= m;
            _state ^= _state >> 15;
            return _state;
        }

        inline machine_word _end(detail::identity<64>) {
            mw len = _len;
            mmix(_state, len)
            _state ^= _state >> r;
            _state *= m;
            _state ^= _state >> r;
            return _state;
        }
#undef mmix
    };
}