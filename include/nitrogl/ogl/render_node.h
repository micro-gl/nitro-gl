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
#include "vbo.h"
#include "ebo.h"
#include "vao.h"
#include "../math/mat4.h"
#include "../color.h"

namespace nitrogl {

    template<class ShaderProgram>
    class render_node {
    public:
        using shader_program_type = ShaderProgram;

    protected:
        const shader_program_type _program;
        vao_t _vao;
//        vbo_t _vbo;
        ebo_t _ebo;
        mat4f _mat_model, _mat_view, _mat_proj;

    public:
        render_node() : render_node(shader_program_type()) {}
        explicit render_node(main_shader_program && program ) noexcept :
            _program(nitrogl::traits::move(program)), _vao(), _ebo(),
            _mat_model(), _mat_view(), _mat_proj() {
        }
        explicit render_node(const shader_program_type & program) :
                    _program(program), _vao(), _ebo(),
                    _mat_model(), _mat_view(), _mat_proj() {
        }
        ~render_node()=default;

        const shader_program_type & program() const { return _program; }
        void updateModelMatrix(const mat4f & from) { _mat_model = from; }
        void updateViewMatrix(const mat4f & from) { _mat_view = from; }
        void updateProjMatrix(const mat4f & from) { _mat_proj = from; }

        void init() {
        }

    };

}