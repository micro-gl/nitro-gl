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
#include "nitrogl/samplers/sampler.h"

namespace nitrogl {

    class shader_compositor {
    public:
        shader_compositor()=delete;
        shader_compositor & operator=(const shader_compositor &)=delete;
        shader_compositor operator=(shader_compositor &&)=delete;
        ~shader_compositor()=delete;

        /**
         * compose a linked main_shader_program with shader with sampler_t
         * @tparam Sampler a sampler_t object type
         * @param sampler sampler_t reference
         * @return a linked program
         */
        static main_shader_program composite_main_program_from_sampler(sampler_t & sampler) {
            main_shader_program prog;
            auto vertex = shader::from_vertex(main_shader_program::vert);
            // fragment shards
            const auto * sampler_main = sampler.main();
            if(*sampler_main=='\n') ++sampler_main;
            const GLchar * sources[7] = { main_shader_program::frag_version,
                                          main_shader_program::frag_other, // uniforms/atrribs/functions
                                          sampler.uniforms(),
                                          sampler.other_functions(),
                                         "vec4 __internal_sample", // main sample function
                                         sampler.main(),
                                         main_shader_program::frag_main};
            auto fragment = shader::from_fragment(sources, 7, nullptr);
            prog.attach_shaders(nitrogl::traits::move(vertex),
                                nitrogl::traits::move(fragment));
            prog.resolve_vertex_attributes_and_uniforms_and_link();
            // sampler_t can now cache uniforms variables
            sampler.on_cache_uniforms_locations(prog.id());
            return prog;
        }

    };

}