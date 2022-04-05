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

#include "../traits.h"

namespace nitrogl {

    /**
     * is char array empty
     * 1. is nullptr
     * 2. up to max length(-1=infinite),
     * 3. up to first null termination char
     */
    inline bool is_empty(const char * a, int max_length=-1) {
        if(a==nullptr) return true;
        for (; max_length; ++a, --max_length) {
            const auto c = *a;
            if(c=='\0') break;
            if(c>=' ' and c<='~') return false;
        }
        return true;
    }

    inline bool is_equal(const char * a, const char * b, int max_length=-1) {
        for (; max_length and *a and *b and *a==*b; ++a, ++b, --max_length) {}
        if(*a=='\0' or max_length==0) return true;
        return false;
    }

    inline const char * index_of_in(const char * a, const char * b, int max_length_of_a=-1) {
        for (int ix=0; *(b+max_length_of_a); ++b, ++ix) {
            const bool is_ = is_equal(a, b, max_length_of_a);
            if(is_) return (b);
        }
        return nullptr;
    }

    inline const char * find_first_not_of_in(const char * in, char v, int max_length_of_a=-1) {
        for (; max_length_of_a and *in and *in==v ; ++in, --max_length_of_a) {}
        return *in!=v ? in : nullptr;
    }

    /**
     * convert char array to signed int. computation will stop when:
     * 1. a non number char is encountered
     * 2. max len amount of number chars is processed
     */
    inline int s2i(const char *c, int max_len) {
        int s = 1, res = 0;
        if (c[0] == '-') { s = -1; ++c; }
        for (; max_len and (*c >= '0') and (*c <= '9') ; --max_len, ++c) {
            res = res*10 + (*c - '0');
        }
        return res*s;
    }

    inline uint32_t digits10(uint64_t v) {
        uint32_t result = 1;
        for (;; v /= 10000U, result+=4) {
            if (v < 10) return result;
            if (v < 100) return result + 1;
            if (v < 1000) return result + 2;
            if (v < 10000) return result + 3;
        }
    }

    inline size_t facebook_uint32_to_str(uint32_t value, char *dst) {
        static const char digits[201] =
                "0001020304050607080910111213141516171819"
                "2021222324252627282930313233343536373839"
                "4041424344454647484950515253545556575859"
                "6061626364656667686970717273747576777879"
                "8081828384858687888990919293949596979899";
        size_t const length = digits10(value);
        size_t next = length - 1;
        while (value >= 100) {
            auto const i = (value % 100) * 2;
            value /= 100;
            dst[next] = digits[i + 1];
            dst[next - 1] = digits[i];
            next -= 2;
        }
        // Handle last 1-2 digits
        if (value < 10) {
            dst[next] = '0' + uint32_t(value);
        } else {
            auto i = uint32_t(value) * 2;
            dst[next] = digits[i + 1];
            dst[next - 1] = digits[i];
        }
        return length;
    }

    class numbers_99_db {
        static const char * db() {
            return "0001020304050607080910111213141516171819"
                   "2021222324252627282930313233343536373839"
                   "4041424344454647484950515253545556575859"
                   "6061626364656667686970717273747576777879"
                   "8081828384858687888990919293949596979899";
        }

    public:
        static const char * get(unsigned int ix) {
            return db() + (ix<<1);
        }
    };

}