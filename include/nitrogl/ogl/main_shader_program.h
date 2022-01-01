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

// matrix of model from local to world space
uniform mat4 mat_model;
// matrix of projection from world into clip space
uniform mat4 mat_projection;
// uv coords of blended texture
uniform vec2 uvs_backdrop_texture[4];

// index of vertex
in float index;
// position of vertex
in vec2 position;
// uv of vertex
in vec3 tex_vertex_in;

// uv coord of main shape
out vec3 tex_coord_0;
// uv coord of blended texture
out vec2 tex_coord_backdrop;

void main()
{
    int idx = int(index);
    // correct tex coords of main texture
    tex_coord_0 = tex_vertex_in;
    // correct tex coords of blended texture
    tex_coord_backdrop = vec2(uvs_backdrop_texture[idx].x, uvs_backdrop_texture[idx].y);

    // output vertex position in clip space after transforming
    gl_Position = mat_projection * mat_model * vec4(position, 0.0, 1.0);
}
)foo";

        static constexpr const char * const frag = R"foo(
#version 330 core
out vec4 outColor;
void main()
{
    outColor = vec4(0.0, 0.5, 0.25, 1.0);
}
        )foo";
    public:

        // ctor: init with empty shaders and attach which is legal
        main_shader_program() :
                shader_program(shader_program::from_shaders(shader::from_vertex(vert),
                                                            shader::from_fragment(frag))) {

        }

        ~main_shader_program() = default;

        void updateModelMatrix(nitrogl::mat4f & matrix) {
            updateUniformMatrix4fv("mat_model", matrix.data());
        }

        void updateProjectionMatrix(nitrogl::mat4f & matrix) {
            updateUniformMatrix4fv("mat_projection", matrix.data());
        }

        void updateTextureSampler(const GLchar * name, GLint texture_index) {
            updateUniform1i(name, texture_index);
        }

        void updateOpacity(GLfloat opacity) {
            updateUniform1f("opacity", opacity);
        }

        void updateColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
            updateUniform4f("color", r, g, b, a);
        }

    };

}