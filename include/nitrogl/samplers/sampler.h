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
        virtual sampler_t * on_sub_sampler_request(unsigned index) { return nullptr; }

    protected:
        struct no_more_than_999_samplers_allowed {};
        const unsigned int _id;
        unsigned int _sub_samplers_count=0;
        char _id_string[4]; // plus null termination

        sampler_t() : _id(assign_id()), _id_string{0} {
            const auto len = nitrogl::facebook_uint32_to_str(_id, _id_string);
            if(len>=3) {
#ifdef NITROGL_ENABLE_THROW
                throw no_more_than_999_samplers_allowed();
#endif
            }
        }

    public:
        unsigned sub_samplers_count () const { return _sub_samplers_count; };
        sampler_t * sub_sampler(unsigned index) {
            return on_sub_sampler_request(index);
        }
        unsigned int id() const { return _id; }
        const char * id_string() const { return _id_string; }
        virtual const char * name() { return ""; };
        virtual const char * uniforms() { return nullptr; }
        virtual const char * other_functions() { return "\n"; }
        virtual const char * main() = 0;
        virtual void on_cache_uniforms_locations(GLuint program) {};
        virtual void on_upload_uniforms() {}
        virtual ~sampler_t()=default;
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