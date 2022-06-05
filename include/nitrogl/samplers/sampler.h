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
#include "../_internal/string_utils.h"
#include "../_internal/murmur.h"

namespace nitrogl {

    struct sampler_t {
    private:
        struct traversal_info_t {
            int id;
            bool visited;

            const char * id_str() const {
                return nitrogl::numbers_99_db::get(id);
            }
            static constexpr char size_id_str() { return 2; }
        };

    protected:
        struct no_more_than_999_samplers_allowed {};
        struct no_more_than_99_samplers_allowed {};
        struct location_of_uniform_not_found {};
        unsigned int _sub_samplers_count;

        sampler_t() : _sub_samplers_count(0), _traversal_info{-1, false},
                        intrinsic_width(0.0f), intrinsic_height(0.0f) {
        }

    public:
        float intrinsic_width;
        float intrinsic_height;
        traversal_info_t _traversal_info;
        traversal_info_t & traversal_info() {
            return _traversal_info;
        }
        GLint get_uniform_location(GLuint program, const char * name) const {
            static char s[50] {0};
            auto i = _traversal_info.id_str();
            s[0]='d';s[1]='a';s[2]='t';s[3]='a';s[4]='_';
            s[5]=i[0];s[6]=i[1];
            char * next = s + 5 + _traversal_info.size_id_str();
            *(next++) = '.';
            for (;; ++name, ++next) {
                const auto c = *name;
                if(!c) break; // test null termination
                *next=c;
            }
            *next='\0'; // add null termination
            const auto loc = glGetUniformLocation(program, s);
#ifdef NITROGL_ENABLE_THROW
            if(loc==-1) throw location_of_uniform_not_found();
#endif
            return loc;
        }
        virtual ~sampler_t()=default;
        unsigned sub_samplers_count () const { return _sub_samplers_count; };
        sampler_t * sub_sampler(unsigned index) const {
            return sub_samplers()[index];
        }
        sampler_t * sub_sampler(unsigned index) {
            return sub_samplers()[index];
        }
        virtual const char * name() const { return ""; };
        virtual const char * uniforms() const {
            return nullptr;
        }
        virtual const char * other_functions() const { return nullptr; }
        virtual const char * main() const = 0;
        void cache_uniforms_locations(GLuint program) {
            const auto ssc = sub_samplers_count();
            for (unsigned ix = 0; ix < ssc; ++ix)
                sub_sampler(ix)->cache_uniforms_locations(program);
            on_cache_uniforms_locations(program);
        };
        void upload_uniforms(GLuint program) {
            const auto ssc = sub_samplers_count();
            for (unsigned ix = 0; ix < ssc; ++ix)
                sub_sampler(ix)->upload_uniforms(program);
            on_upload_uniforms_request(program);
        };

        virtual nitrogl::uintptr_type hash_code() const {
            microc::iterative_murmur<nitrogl::uintptr_type> murmur;
            murmur.begin_cast(main());
            const auto ssc = sub_samplers_count();
            for (unsigned int ix = 0; ix < ssc; ++ix)
                murmur.next(sub_sampler(ix)->hash_code());
            return murmur.end();
        }

        virtual sampler_t * const * sub_samplers() const { return nullptr; }
        virtual sampler_t ** sub_samplers() { return nullptr; }
        virtual void on_cache_uniforms_locations(GLuint program) {};
        virtual void on_upload_uniforms_request(GLuint program) {}
        virtual unsigned int generate_traversal(unsigned int id) {
            _traversal_info.id=id;
            _traversal_info.visited=false;
            if(id > 99) {
#ifdef NITROGL_ENABLE_THROW
                throw no_more_than_99_samplers_allowed();
#endif
            }
            return ++id;
        }
    };

    template<unsigned N>
    struct multi_sampler : public sampler_t {
    protected:
        sampler_t * _sub_samplers[N];

    public:
        sampler_t * const * sub_samplers() const override {
            return _sub_samplers;
        }
        sampler_t ** sub_samplers() override {
            return _sub_samplers;
        }

        multi_sampler() : _sub_samplers{nullptr}, sampler_t() {
            _sub_samplers_count=N;
        }

        template <class... Ts>
        multi_sampler(Ts... rest) : _sub_samplers{rest...}, sampler_t() {
            _sub_samplers_count=N;
        }

        multi_sampler & add_sub_sampler(sampler_t * sampler) {
            _sub_samplers[_sub_samplers_count++] = sampler;
            return *this;
        }

        unsigned int generate_traversal(unsigned int id) override {
            const auto ssc = sub_samplers_count();
            for (unsigned int ix = 0; ix < ssc; ++ix)
                id = sub_sampler(ix)->generate_traversal(id);
            return sampler_t::generate_traversal(id);
        }

    };
}