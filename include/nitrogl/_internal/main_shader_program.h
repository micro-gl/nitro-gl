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

#include "nitrogl/ogl/shader_program.h"
#include "nitrogl/math/mat4.h"

namespace nitrogl {

    class main_shader_program : public shader_program {
    public:
        static constexpr const char * const vert = R"foo(
#version 330 core

// uniforms
uniform mat4 mat_model;
uniform mat4 mat_view;
uniform mat4 mat_proj;
uniform mat3 mat_transform_uvs;

// in = vertex attributes
in vec2 VS_pos; // position of vertex
in vec4 VS_uvs_sampler; // uv of vertex, extras will be taken from (0, 0, 0, 1) if vbo input is smaller

// out = varying
out vec3 PS_uvs_sampler;

void main()
{
    PS_uvs_sampler = vec3((mat_transform_uvs * vec3(VS_uvs_sampler.st, 1.0)).st,
                           VS_uvs_sampler.q); // remove 2nd component from VS_uvs_sampler
    gl_Position = mat_proj * mat_view * mat_model * vec4(VS_pos, 1.0, 1.0);
}
)foo";

        constexpr static const char * const frag_version = "#version 330 core\n";

        constexpr static const char * const define_sampler = "#define __SAMPLER_MAIN ";

        constexpr static const char * const frag_other = R"foo(
//#version 330 core

// uniforms
//uniform vec4 color; // color

// in
in vec3 PS_uvs_sampler;

// out
out vec4 FragColor;

vec4 sample1(vec3 uv) {
    return vec4(uv.x, uv.x, uv.x, 1.0);
}
)foo";

        constexpr static const char * const frag_blend = "vec3 __blend_color";
        constexpr static const char * const frag_composite = "vec4 __composite_alpha";

        constexpr static const char * const frag_main = R"foo(
uniform struct DATA { float aa; } tomer_0;

void main()
{
//    TTT tomer_0;
//    tomer_0.aa=2.0;
//    vec4 source = __internal_sample(PS_uvs_sampler, 0);
//    vec4 target = __internal_sample(PS_uvs_sampler, 0);
//    vec3 _blend = __blend_color
    FragColor = __SAMPLER_MAIN(PS_uvs_sampler, 0);
//    FragColor = color;
}
        )foo";

    public:

        struct VAS {
            shader_program::shader_vertex_attr_t data[2];
            static constexpr unsigned size() { return 2; }
        };

        // I have to have this uniform location cache. It is different
        // from shader to shader instance, so I have no way around saving it.
        struct uniforms_type {
            GLint mat_model=-1, mat_view=-1, mat_proj=-1, mat_transform_uvs=-1, opacity=-1;
        };

        uniforms_type uniforms;

        const uniforms_type & uniforms_locations() const {
            return uniforms;
        }
        static const VAS & shader_vertex_attributes() {
            // this is static because we know their locations is const and predictable,
            // so it does not change between instances of this shader. This saves on memory
            // requirements of this class so this is a win-win. putting this as static inside
            // a method is atrick to avoid define it once in a separate cpp file
            constexpr static VAS vas = {{
                {"VS_pos", 0,
                 shader_program::shader_attribute_component_type::Float},
                 {"VS_uvs_sampler", 1,
                  shader_program::shader_attribute_component_type::Float}
            }};
            return vas;
        }

        // ctor: init with empty shaders and attach which is legal
        main_shader_program() : uniforms(), shader_program() {}
        main_shader_program(bool test) : uniforms(), shader_program() {
            const GLchar * frag_shards[3] = { frag_version, frag_other, frag_main };
            auto v = shader::from_vertex(vert);
            auto f = shader::from_fragment(frag_shards, 3, nullptr);
            attach_shaders(nitrogl::traits::move(v), nitrogl::traits::move(f));
            resolve_vertex_attributes_and_uniforms_and_link();
        }
        main_shader_program(const main_shader_program & o) = default;
        main_shader_program(main_shader_program && o) noexcept : shader_program(nitrogl::traits::move(o)), uniforms(o.uniforms) {}
        main_shader_program & operator=(const main_shader_program & o) = default;
        main_shader_program & operator=(main_shader_program && o)  noexcept {
            shader_program::operator=(nitrogl::traits::move(o));
            uniforms=o.uniforms;return *this;
        }

        ~main_shader_program() = default;

        void resolve_vertex_attributes_and_uniforms_and_link() {
            // first set vertex attributes locations via binding, in case we are not using location qualifiers
            setVertexAttributesLocations(shader_vertex_attributes().data, shader_vertex_attributes().size());
            // program should be linked by previous call to set, but in case we have zero attributes, make sure
            if(!wasLastLinkSuccessful()) link();
            // cache uniform locations after link
            uniforms.mat_model = uniformLocationByName("mat_model");
            uniforms.mat_view = uniformLocationByName("mat_view");
            uniforms.mat_proj = uniformLocationByName("mat_proj");
            uniforms.mat_transform_uvs = uniformLocationByName("mat_transform_uvs");
            uniforms.opacity = uniformLocationByName("opacity");
        }

    public:
        void updateModelMatrix(const nitrogl::mat4f & matrix) const
        {  glUniformMatrix4fv(uniforms.mat_model, 1, GL_FALSE, matrix.data()); }
        void updateViewMatrix(const nitrogl::mat4f & matrix) const
        {  glUniformMatrix4fv(uniforms.mat_view, 1, GL_FALSE, matrix.data()); }
        void updateProjectionMatrix(const nitrogl::mat4f & matrix) const
        {  glUniformMatrix4fv(uniforms.mat_proj, 1, GL_FALSE, matrix.data()); }
        void updateUVsTransformMatrix(const nitrogl::mat3f & matrix) const
        {  glUniformMatrix3fv(uniforms.mat_transform_uvs, 1, GL_FALSE, matrix.data()); }
//        void updateTextureSampler(const GLchar * name, GLint texture_index) const
//        { glUniform1i(name, texture_index); }
        void updateOpacity(GLfloat opacity) const
        { glUniform1f(uniforms.opacity, opacity); }

    };

}