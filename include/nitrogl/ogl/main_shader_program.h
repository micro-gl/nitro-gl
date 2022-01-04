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

#include "shader_program.h"
#include "../math/mat4.h"

namespace nitrogl {

    class main_shader_program : public shader_program {
        static constexpr const char * const vert = R"foo(
#version 330 core

// uniforms
uniform mat4 mat_model;
uniform mat4 mat_view;
uniform mat4 mat_proj;

// in = vertex attributes
in vec2 VS_pos; // position of vertex
in vec3 VS_uvs_sampler; // uv of vertex

// out = varying
out vec3 PS_uvs_sampler;

void main()
{
//    mat3 aa(1.0f);
    PS_uvs_sampler = VS_uvs_sampler;
    gl_Position = mat_proj * mat_view * mat_model * vec4(VS_pos, 1.0, 1.0);
}
)foo";

        static constexpr const char * const frag = R"foo(
#version 330 core

// uniforms
uniform vec4 color; // color

// out
out vec4 FragColor;

void main()
{
    FragColor = color;
}
        )foo";
    public:

        // ctor: init with empty shaders and attach which is legal
        main_shader_program() :
                shader_program(shader::from_vertex(vert), shader::from_fragment(frag)) {
        }
        main_shader_program(const main_shader_program & o) : shader_program(o) {}
        main_shader_program(main_shader_program && o) noexcept : shader_program(nitrogl::traits::move(o)) {}
        main_shader_program & operator=(const main_shader_program & o) {
            shader_program::operator=(o); return *this;
        }
        main_shader_program & operator=(main_shader_program && o)  noexcept {
            shader_program::operator=(nitrogl::traits::move(o)); return *this;
        }

        ~main_shader_program() = default;

        void updateModelMatrix(nitrogl::mat4f & matrix) const
        { updateUniformMatrix4fv("mat_model", matrix.data()); }
        void updateViewMatrix(nitrogl::mat4f & matrix) const
        { updateUniformMatrix4fv("mat_view", matrix.data()); }
        void updateProjectionMatrix(nitrogl::mat4f & matrix) const
        { updateUniformMatrix4fv("mat_proj", matrix.data()); }
        void updateTextureSampler(const GLchar * name, GLint texture_index) const
        { updateUniform1i(name, texture_index); }
        void updateOpacity(GLfloat opacity) const
        { updateUniform1f("opacity", opacity); }
        void updateColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) const
        { updateUniform4f("color", r, g, b, a); }

    };

}