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

#include "../ogl/shader_program.h"
#include "../math/mat4.h"

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
uniform vec4 bbox;
uniform bool has_missing_uvs;
uniform bool has_missing_q;

// in = vertex attributes
in vec2 VS_pos; // position of vertex
in vec2 VS_uvs_sampler; // uv of vertex, extras will be taken from (0, 0, 0, 1) if vbo input is smaller
in float VS_q_sampler; // q of vertex, good for projections

// out = varying
out vec3 PS_uvs_sampler;

void main()
{
    // Uniform Branching in vertex shader is OK, not a bottleneck
    float q = has_missing_q ? 1.0 : VS_q_sampler;
    // missing uv
    vec2 uv_missing = (VS_pos - bbox.xy)/bbox.zw;
//    vec2 uv_missing = (VS_pos - bbox.xy)/vec2(256,256);
    uv_missing.y = 1.0 - uv_missing.y;
    // final uv
    vec2 uv = has_missing_uvs ? uv_missing : VS_uvs_sampler;
    PS_uvs_sampler = vec3((mat_transform_uvs * vec3(uv, 1.0)).st, q);
    gl_Position = mat_proj * mat_view * mat_model * vec4(VS_pos, 1.0, 1.0);
}

)foo";

        constexpr static const char * const frag_version = "#version 330 core\n";

        constexpr static const char * const define_sampler = "#define __SAMPLER_MAIN sampler_";
        constexpr static const char * const define_premul_alpha = "\n#define __PRE_MUL_ALPHA\n";

        constexpr static const char * const frag_other = R"foo(
// uniforms

uniform struct DATA_MAIN {
    uint time;
    float opacity;
    sampler2D texture_backdrop;
    uvec2 window_size;
} data_main;

// in
in vec3 PS_uvs_sampler;
//layout(origin_upper_left) in vec4 gl_FragCoord;

// out
out vec4 FragColor;

//vec4 sample1(vec3 uv) {
//    return vec4(uv.x, uv.x, uv.x, 1.0);
//}
)foo";

        constexpr static const char * const frag_blend = "vec3 __blend_color";
        constexpr static const char * const frag_composite = "vec4 __composite_alpha";

        constexpr static const char * const alpha_mul = R"(
)";
        constexpr static const char * const frag_main = R"foo(

// inplace: Cs = (1 - αb) x Cs + αb x B(Cb, Cs)
vec4 __blend_in_place(in vec4 src, in vec4 backdrop, in vec3 B) {
    vec4 result;
    result.rgb = (1.0 - backdrop.a) * src.rgb + backdrop.a * clamp(B, 0.0, 1.0);
    result.a = src.a;
    return result;
}

void main()
{
    // get backdrop uvs
    // coords are screen space left to right, bottom is 0, top is 1.
    vec2 bd_uvs = vec2(gl_FragCoord.x, gl_FragCoord.y)/data_main.window_size;

// these are inverted
//    vec2 bd_uvs = vec2(gl_FragCoord.x, data_main.window_size.y-gl_FragCoord.y)/data_main.window_size;
//    vec2 coords = (gl_FragCoord.xy)/data_main.window_size;

    // sample from backdrop
    vec4 bd_texel = texture(data_main.texture_backdrop, bd_uvs);
    // un mul alpha if backdrop is alpha-mul
#ifdef __PRE_MUL_ALPHA
    bd_texel.rgb /= bd_texel.a;
#endif

    // sample from un-multiplied-alpha sampler, also, perspective correct the uvs with q coord
    vec4 sampler_out = __SAMPLER_MAIN(PS_uvs_sampler/PS_uvs_sampler.z);
    // apply opacity
    sampler_out.a *= data_main.opacity;
    // blend mode with un-multiplied-alpha backdrop
    vec3 blended_colors_only = __BLEND(sampler_out.rgb, bd_texel.rgb);
    vec4 blended_colors_final = __blend_in_place(sampler_out, bd_texel, blended_colors_only);
    // alpha composite with backdrop --> result is ALPHA-MULTIPLIED
    vec4 composited = __COMPOSITE(blended_colors_final, bd_texel);
    FragColor = composited;

#ifndef __PRE_MUL_ALPHA
    FragColor.rgb /= FragColor.a;
#endif

//    FragColor =vec4(coords, 0, 1.0);
//    FragColor = vec4(bd_uvs.x, bd_uvs.x, bd_uvs.x, 1.0);
//    FragColor = vec4(bd_uvs.y, bd_uvs.y, bd_uvs.y, 1.0);
}
        )foo";

    public:

        struct VAS {
            shader_program::shader_vertex_attr_t data[3];
            static constexpr unsigned size() { return 3; }
        };

        // I have to have this uniform location cache. It is different
        // from shader to shader instance, so I have no way around saving it.
        struct uniforms_type {
            GLint mat_model=-1, mat_view=-1, mat_proj=-1, mat_transform_uvs=-1,
            bbox=-1, has_missing_uvs=-1, has_missing_q=-1,
            opacity=-1, time=-1, tex_backdrop=-1, window_size=-1;
        };

        uniforms_type uniforms;

        const uniforms_type & uniforms_locations() const {
            return uniforms;
        }
        static const VAS & shader_vertex_attributes() {
            // this is static because we know their locations is const and predictable,
            // so it does not change between instances of this shader. This saves on memory
            // requirements of this class so this is a win-win. putting this as static inside
            // a method is a trick to avoid define it once in a separate cpp file
            static VAS vas = {{
                {"VS_pos", 0,
                 shader_program::shader_attribute_component_type::Float},
                {"VS_uvs_sampler", 1,
                  shader_program::shader_attribute_component_type::Float},
                {"VS_q_sampler", 2,
                   shader_program::shader_attribute_component_type::Float},
            }};
            return vas;
        }

        // ctor: internal_init with empty shaders and attach which is legal
        main_shader_program(const shader & vertex, const shader & fragment, bool $link=false) :
                            shader_program(vertex, fragment, $link), uniforms() {
        }
        main_shader_program(shader && vertex, shader && fragment, bool $link=false) :
                    shader_program(nitrogl::traits::move(vertex),
                                   nitrogl::traits::move(fragment), $link), uniforms() {
        }
        main_shader_program() : uniforms(), shader_program() {} // with empty shaders
        main_shader_program(bool test) : uniforms(), shader_program() {
            const GLchar * frag_shards[3] = { frag_version, frag_other, frag_main };
            auto v = shader::from_vertex(vert);
            auto f = shader::from_fragment(frag_shards, 3, nullptr);
            update_shaders(nitrogl::traits::move(v), nitrogl::traits::move(f));
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
            // cache base uniform locations after link
            uniforms.mat_model = uniformLocationByName("mat_model");
            uniforms.mat_view = uniformLocationByName("mat_view");
            uniforms.mat_proj = uniformLocationByName("mat_proj");
            uniforms.mat_transform_uvs = uniformLocationByName("mat_transform_uvs");
            uniforms.has_missing_uvs = uniformLocationByName("has_missing_uvs");
            uniforms.has_missing_q = uniformLocationByName("has_missing_q");
            uniforms.bbox = uniformLocationByName("bbox");

            uniforms.opacity = uniformLocationByName("data_main.opacity");
            uniforms.time = uniformLocationByName("data_main.time");
            uniforms.tex_backdrop = uniformLocationByName("data_main.texture_backdrop");
            uniforms.window_size = uniformLocationByName("data_main.window_size");
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
        void updateBBox(float left, float top, float right, float bottom) const
        {  glUniform4f(uniforms.bbox, left, top, right-left, bottom-top); }
        void update_has_missing_uvs(bool value) const
        {  glUniform1i(uniforms.has_missing_uvs, value); }
        void update_has_missing_qs(bool value) const
        {  glUniform1i(uniforms.has_missing_q, value); }
        void updateOpacity(GLfloat opacity) const
        { glUniform1f(uniforms.opacity, opacity); }
        void update_time(GLuint value) const
        { glUniform1ui(uniforms.time, value); }
        void update_backdrop_texture(const gl_texture & texture) const {
            const auto unit = 0;//gl_texture::next_texture_unit();
            texture.use(unit);
            glUniform1i(uniforms.tex_backdrop, unit);
        }
        void update_window_size(GLuint w, GLuint h) const {
            glUniform2ui(uniforms.window_size, w, h);
        }

    };

}