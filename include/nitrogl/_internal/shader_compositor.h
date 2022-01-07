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
#include "nitrogl/_internal/main_shader_program.h"

namespace nitrogl {

    class shader_compositor {
    public:
        shader_compositor()=delete;
        ~shader_compositor()=delete;

        template<class Sampler>
        static main_shader_program composite_un_linked_shader_from_sampler(const Sampler & sampler) {
            main_shader_program prog;
            auto vertex = shader::from_vertex(main_shader_program::vert);
            // fragment shards
            const GLchar * sources[6] = { main_shader_program::frag0,
                                         sampler.other(),
                                         "vec4 __internal_sample",
                                         sampler.main(),
                                         main_shader_program::frag1,
                                         main_shader_program::frag2 };
            auto fragment = shader::from_fragment(sources, 6, nullptr);
            prog.attach_shaders(nitrogl::traits::move(vertex),
                                nitrogl::traits::move(fragment));
            return prog;
        }

    };

}