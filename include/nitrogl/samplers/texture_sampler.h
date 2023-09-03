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

#include <nitrogl/samplers/sampler.h>
#include <nitrogl/traits.h>
#include <nitrogl/ogl/gl_texture.h>

namespace nitrogl {

    struct texture_sampler : public sampler_t {
        const char * name() const override { return "texture_sampler"; }
        const char * uniforms() const override {
            return R"(
{
    sampler2D texture;
}
)";
        }

        nitrogl::uintptr_type hash_code() const override {
            // on some opengl hardware, when the slot uniform changes,
            // the driver patches or recompiles the shader because texture is
            // a non-opaque data type. Therefore, slot MUST change the hash code
            // of the sampler. Therefore, EVERY UNIQUE SLOT HAS TO GENERATE A UNIQUE SHADER !!!
            // Otherwise, performance will be very bad if user uses the same shader with dynamic
            // slot uniform, this will cause patching --> very bad performance
            microc::iterative_murmur<nitrogl::uintptr_type> murmur;
            murmur.begin_cast(main());
            murmur.next(texture.slot());
            return murmur.end();
        }

        const char * main() const override {
            if(texture.is_premul_alpha())
                return R"(
(in vec3 uv) {
    vec4 tex = TEXTURE_2D(data.texture, uv.xy);
    tex.rgb/=tex.a;
    return clamp(tex, 0.0, 1.0);
}
)";
            else
                return R"(
(in vec3 uv) {
    return TEXTURE_2D(data.texture, uv.xy);
}
)";
        }

        void on_cache_uniforms_locations(GLuint program) override {
        }

        void on_upload_uniforms_request(GLuint program) override {
            texture.use(texture.slot());
            glUniform1i(get_uniform_location(program, "texture"), texture.slot());
        }

        void update_intrinsic(bool on) {
            intrinsic_width = on ? float(texture.width()) : -1.0f;
            intrinsic_height = on ? float(texture.height()) : -1.0f;
        }

        gl_texture texture;
        explicit texture_sampler(const gl_texture & texture,
                                 bool intrinsic=false) :
                texture(texture), sampler_t() {
            update_intrinsic(intrinsic);
        }
        explicit texture_sampler(gl_texture && texture,
                                 bool intrinsic=false) :
                texture(nitrogl::traits::move(texture)), sampler_t() {
            update_intrinsic(intrinsic);
        }
    };
}