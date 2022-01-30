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

#include "sampler.h"
#include "../traits.h"
#include "../ogl/gl_texture.h"

namespace nitrogl {

    template<bool is_texture_pre_mul_alpha=true>
    struct texture_sampler : public sampler_t {
        const char * name() const override { return "texture_sampler"; }
        const char * uniforms() const override {
            return R"(
{
    sampler2D texture;
}
)";
        }

        const char * main() const override {
            if(is_texture_pre_mul_alpha)
                return R"(
(vec3 uv) {
    vec4 tex = texture(data.texture, uv.xy/uv.z);
    return vec4(tex.rgb/tex.a, tex.a);
}
)";
            else
                return R"(
(vec3 uv) {
    return texture(data.texture, uv.xy/uv.z);
}
)";
        }

        void on_cache_uniforms_locations(GLuint program) override {
        }

        void on_upload_uniforms_request(GLuint program) override {
            const auto tex_unit = gl_texture::next_texture_unit();
            glUniform1i(get_uniform_location(program, "texture"), tex_unit);
            texture.use(tex_unit);
        }

    public:
        gl_texture texture;
        explicit texture_sampler(const gl_texture & texture) : texture(texture), sampler_t() {}
        explicit texture_sampler(gl_texture && texture) : texture(nitrogl::traits::move(texture)), sampler_t() {}
    };
}