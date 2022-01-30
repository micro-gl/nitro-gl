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

namespace nitrogl {

    struct sampler_t {
    private:
        static unsigned assign_id() {
            static unsigned int id=-1;
            return ++id;
        }

    protected:
        struct no_more_than_999_samplers_allowed {};
        const unsigned int _id;
        unsigned int _sub_samplers_count=0;
        char _id_string[4]; // "###0"
        char _id_string_len; // "###0"

        sampler_t() : _id(assign_id()), _id_string{0}, _id_string_len(0) {
            _id_string_len = nitrogl::facebook_uint32_to_str(_id, _id_string);
            if(_id_string_len>3) {
#ifdef NITROGL_ENABLE_THROW
                throw no_more_than_999_samplers_allowed();
#endif
            }
        }
        static char * get_general_static_string_storage() {
            // used for assembling strings for uniforms locations
            static char storage[100]{0};
            return storage;
        }

    public:
        GLint get_uniform_location(GLuint program, const char * name) const {
            char s[50] {0};
            auto i = _id_string;
            s[0]='d';s[1]='a';s[2]='t';s[3]='a';s[4]='_';
            s[5]=i[0];s[6]=i[1];s[7]=i[2];s[8]=i[3];s[9]=i[4];
            char * next = s + 5 + _id_string_len;
            *(next++) = '.';
            for (;; ++name, ++next) {
                const auto c = *name;
                if(!c) break; // test null termination
                *next=c;
            }
            *next='\0'; // add null termination
            return glGetUniformLocation(program, s);
        }
        virtual ~sampler_t()=default;
        unsigned sub_samplers_count () const { return _sub_samplers_count; };
        sampler_t * sub_sampler(unsigned index) {
            return on_sub_sampler_request(index);
        }
        unsigned int id() const { return _id; }
        const char * id_string() const { return _id_string; }
        virtual const char * name() const { return ""; };
        virtual const char * uniforms() const { return nullptr; }
        virtual const char * other_functions() const { return nullptr; }
        virtual const char * main() const = 0;
        void cache_uniforms_locations(GLuint program) {
            const auto ssc = sub_samplers_count();
            for (int ix = 0; ix < ssc; ++ix)
                sub_sampler(ix)->cache_uniforms_locations(program);
            on_cache_uniforms_locations(program);
        };
        void upload_uniforms(GLuint program) {
            const auto ssc = sub_samplers_count();
            for (int ix = 0; ix < ssc; ++ix)
                sub_sampler(ix)->upload_uniforms(program);
            on_upload_uniforms_request(program);
        };

    protected:
        virtual sampler_t * on_sub_sampler_request(unsigned index) { return nullptr; }
        virtual void on_cache_uniforms_locations(GLuint program) {};
        virtual void on_upload_uniforms_request(GLuint program) {}
    };

    template<unsigned N>
    struct multi_sampler : public sampler_t {
    private:
        sampler_t * _sub_samplers[N];

    protected:
        sampler_t * on_sub_sampler_request(unsigned index) override {
            return _sub_samplers[index];
        }

    public:
        multi_sampler() : _sub_samplers{nullptr}, sampler_t() {}

        void add_sub_sampler(sampler_t * sampler) {
            _sub_samplers[_sub_samplers_count++] = sampler;
        }

    };
}