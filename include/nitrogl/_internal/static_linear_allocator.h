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
#define MICRO_ALLOC_THROW
#ifdef MICRO_ALLOC_DEBUG
#include <iostream>
#endif

namespace micro_alloc {

    /**
     * Static storage, unique by the template arguments.
     * Very convenient for creating fast allocators, that use quick resources such as static
     * memory for rapid prototyping.
     *
     * @tparam uintptr_type unsigned integral that can hold a pointer
     * @tparam Alignment alignment requirement of memory
     * @tparam SizeBytes the size in bytes of the memory
     * @tparam TAG a tag to create different instances
     */
    template<class uintptr_type=unsigned long, unsigned Alignment=8,
            unsigned SizeBytes=1024, unsigned BANK=0>
    class static_storage_t {
    public:
        static_storage_t()=delete;

        using byte = unsigned char;
        using uptr = uintptr_type;

        template<typename P> static P int_to(uptr integer) {
            return reinterpret_cast<P>(integer);
        }
        static uptr ptr_to_int(const void *pointer) {
            return reinterpret_cast<uptr>(pointer);
        }

        inline static uptr align_down(const uptr address, const uptr alignment) {
            uptr a = ~(alignment - 1);
            return (address & a);
        }
        static inline uptr align_up(const uptr address, const uptr alignment) {
            uptr align_m_1 = alignment - 1;
            uptr b = ~align_m_1;
            uptr a = (address + align_m_1);
            uptr c = a & b;
            return c;
        }

        struct memory_info_t {
            byte * start;
            byte * end;
            byte * head;

            uptr size() const { return end-start; }
            uptr used() const { return head-start; }
            uptr free() const { return end-head; }
            void reset() { head=start; }
        };

        static memory_info_t & memory() {
            static byte memory[SizeBytes];
            static byte * start = int_to<byte *>(align_up(ptr_to_int(memory), Alignment));
            static byte * end = int_to<byte *>(align_down(
                    ptr_to_int(memory+SizeBytes), Alignment));
            static byte * relative_head = start;
            static memory_info_t mem_info{ start, end, relative_head };
            return mem_info;
        }
    };

    /**
     * Static Linear Allocator:
     *
     * A small, self-contained and efficient linear allocator that uses static memory,
     * which is determined for uniqueness by the (SizeBytes, BANK) tuple
     *
     * Notes:
     * - Alignment is automatically determined by the sizeof a pointer and is calculated
     *   without alignof operator, which is good for C++11, that lacks support for this feature.
     *
     * @tparam T value type
     * @tparam SizeBytes the size of the bank in bytes
     * @tparam BANK the bank number
     */
    template<typename T=unsigned char, unsigned SizeBytes=1024, unsigned BANK=0>
    class static_linear_allocator {
    private:
        template<class Ty> struct remove_reference      {typedef Ty type;};
        template<class Ty> struct remove_reference<Ty&>  {typedef Ty type;};
        template<class Ty> struct remove_reference<Ty&&> {typedef Ty type;};

        template <class _Tp> inline typename remove_reference<_Tp>::type&&
        move(_Tp&& __t) noexcept
        {
            typedef typename remove_reference<_Tp>::type _Up;
            return static_cast<_Up&&>(__t);
        }
        template <class _Tp> inline _Tp&&
        forward(typename remove_reference<_Tp>::type& __t) noexcept
        { return static_cast<_Tp&&>(__t); }

        template <class _Tp> inline _Tp&&
        forward(typename remove_reference<_Tp>::type&& __t) noexcept
        { return static_cast<_Tp&&>(__t); }

        template<bool B, class TRUE, class FALSE> struct cond { typedef TRUE type; };
        template<class TRUE, class FALSE> struct cond<false, TRUE, FALSE> { typedef FALSE type; };
        static constexpr unsigned int PS = sizeof (void *);
        /**
         * An integral type, that is suitable to hold a pointer address
         */
        using uintptr_type = typename cond<
                PS==sizeof(unsigned short), unsigned short ,
                typename cond<
                PS==sizeof(unsigned int), unsigned int,
                typename cond<
                PS==sizeof(unsigned long), unsigned long, unsigned long long>::type>::type>::type;
        using uptr = uintptr_type;

    public:
        struct out_of_memory_exception {};
        static constexpr uptr Alignment = sizeof (uptr);
        using storage_type = static_storage_t<uintptr_type, Alignment, SizeBytes, BANK>;
        using memory_info = typename storage_type::memory_info_t;
        using value_type = T;
        using size_t = unsigned long;

        template<class U> explicit static_linear_allocator(
                const static_linear_allocator<U, SizeBytes, BANK> &o) noexcept : static_linear_allocator() {
        };
        explicit static_linear_allocator() {
            print_header();
        }

        template<class U, class... Args>
        void construct(U *p, Args &&... args) {
            ::new(p) U(forward<Args>(args)...);
        }

        memory_info & storage() {
            return storage_type::memory();
        }
        memory_info storage() const {
            return storage_type::memory();
        }

        T * allocate(size_t n) {
            auto & info = storage_type::memory();
            uptr size = n * sizeof(T);
            uptr aligned_size = storage_type::align_up(size, Alignment);

            print_header();
            print_stats();
            print_allocation_request(aligned_size);

            // record pointer
            auto * pointer = info.head;

            // move head
            info.head = info.head + aligned_size;

            // test for throw
            if(info.head > info.end) {
                print_oom_error();
                throw_oom_if_can();
                return nullptr;
            }

            print_stats();
            print_new_line();
            // else return
            return (T *) pointer;
        }
        void deallocate(T *p, size_t n = 0) {
            // this is a linear allocator, which means it does not deallocate
#ifdef MICRO_ALLOC_DEBUG
            print_header();
            std::cout << "- WARNING: this is a linear allocator, which means it does not deallocate !!!\n";
            print_new_line();
#endif
        }

        void reset() {
            storage().reset();
            print_reset();
        }

        template<class U> struct rebind {
            typedef static_linear_allocator<U, SizeBytes, BANK> other;
        };

        // print and misc stuff, so it won't pollute the readability
        void throw_oom_if_can() {
#ifdef MICRO_ALLOC_THROW
            throw out_of_memory_exception();
#endif
        }
        void print_reset() {
#ifdef MICRO_ALLOC_DEBUG
            print_header();
            std::cout << "- RESET Requested\n";
            print_stats();
            print_new_line();
#endif
        }
        void print_header() {
#ifdef MICRO_ALLOC_DEBUG
            auto info = storage();
            std::cout << "# Static Linear Allocator: " << SizeBytes << " bytes, Bank #" << BANK
            << ", Alignment is " << Alignment << " bytes\n";
#endif
        }
        void print_new_line() {
#ifdef MICRO_ALLOC_DEBUG
            std::cout << std::endl;
#endif
        }
        void print_stats() {
#ifdef MICRO_ALLOC_DEBUG
            auto info = storage();
            std::cout << "- " << info.used() << '/' << info.size() << " bytes used\n";
#endif
        }
        void print_allocation_request(uptr size) {
#ifdef MICRO_ALLOC_DEBUG
            std::cout << "- Allocation request for " << size << " bytes( aligned up )\n";
#endif
        }
        void print_oom_error() {
#ifdef MICRO_ALLOC_DEBUG
            std::cout << "- ERROR: Out Of Memory !!!\n";
#endif
        }

    };

    template<class T1, class T2, class uintptr_type,
            unsigned Alignment, unsigned SizeBytes, unsigned BANK>
    bool operator==(const static_linear_allocator<T1, SizeBytes, BANK> &lhs,
                    const static_linear_allocator<T2, SizeBytes, BANK> &rhs) noexcept {
        // true for the same (SizeBytes, BANK) sequence
        return true;
    }
}