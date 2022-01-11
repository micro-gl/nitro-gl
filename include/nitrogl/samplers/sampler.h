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

    struct sampler_t {
        virtual const char * name() = 0;
        virtual const char * uniforms() { return "\n"; }
        virtual const char * other_functions() { return "\n"; }
        virtual const char * main() = 0;
        virtual void on_cache_uniforms_locations(GLuint program) {};
        virtual void on_upload_uniforms() {}
        virtual ~sampler_t()=default;
    };
}