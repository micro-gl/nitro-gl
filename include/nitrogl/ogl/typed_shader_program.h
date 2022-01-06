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

    template<unsigned UNIFORMS_COUNT, unsigned VAS_COUNT>
    class typed_shader_program : public shader_program {
    public:

        template<unsigned N>
        struct UNIFORMS {
            UNIFORMS()=default;
            uniform_t data[N];
            constexpr unsigned size() const { return N; }
        };

        template<unsigned N>
        struct VAS {
            VAS()=default;
            attr_t data[N];
            constexpr unsigned size() const { return N; }
        };

        UNIFORMS<UNIFORMS_COUNT> uniforms;
        VAS<VAS_COUNT> vas;

        // ctor: init with empty shaders and attach which is legal
        typed_shader_program() : shader_program(), uniforms(), vas() {
            resolveUniformsNames();
        }
        main_shader_program(const main_shader_program & o) = default;
        main_shader_program(main_shader_program && o) noexcept : shader_program(nitrogl::traits::move(o)) {}
        main_shader_program & operator=(const main_shader_program & o) = default;
        main_shader_program & operator=(main_shader_program && o)  noexcept {
            shader_program::operator=(nitrogl::traits::move(o)); return *this;
        }

        ~main_shader_program() = default;

        void resolveUniformsNames() {
            if(!wasLastLinkSuccessful()) link();
            uniforms.mat_model = uniformLocationByName("mat_model");
            uniforms.mat_view = uniformLocationByName("mat_view");
            uniforms.mat_proj = uniformLocationByName("mat_proj");
            uniforms.mat_transform_uvs = uniformLocationByName("mat_transform_uvs");
            uniforms.opacity = uniformLocationByName("opacity");
        }

        void updateModelMatrix(const nitrogl::mat4f & matrix) const
        {  glUniformMatrix4fv(uniforms.mat_model, 1, GL_FALSE, matrix.data()); }
        void updateViewMatrix(const nitrogl::mat4f & matrix) const
        {  glUniformMatrix4fv(uniforms.mat_view, 1, GL_FALSE, matrix.data()); }
        void updateProjectionMatrix(const nitrogl::mat4f & matrix) const
        {  glUniformMatrix4fv(uniforms.mat_proj, 1, GL_FALSE, matrix.data()); }
        void updateUVsTransformMatrix(const nitrogl::mat3f & matrix) const
        {  glUniformMatrix3fv(uniforms.mat_transform_uvs, 1, GL_FALSE, matrix.data()); }
        void updateTextureSampler(const GLchar * name, GLint texture_index) const
        { updateUniform1i(name, texture_index); }
        void updateOpacity(GLfloat opacity) const
        { glUniform1f(uniforms.opacity, opacity); }

    };

}