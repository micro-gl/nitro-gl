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
         * uniform struct DATA_XXX { float a;  vec2 b; } data_XXX;
         */
        static constexpr const GLchar * const struct_0 = "uniform struct DATA_";
        static constexpr const GLchar * const struct_1 = "data_";
        static constexpr const GLchar comma = ';';
        static constexpr const GLchar * const sampler_pre = "vec4 sampler_";
        static constexpr const GLchar * const sampler = "sampler_";
        static constexpr const GLchar * const data = "data.";

        template<int N>
        struct linear_buffer {
            char data[N];
            int head;

            linear_buffer() : data(), head(0) {}

            int write_int(int v) {
                int len = facebook_uint32_to_str(v, data);
                data+=len;
                return len;
            }

            int write_char_array(char * arr) {
                int ix = 0;
                for (; (ix < N-head) and *arr; ++ix, *(data++)=*(arr++)) {
                }
                return ix;
            }

            int write_range(const char * begin, const char * end) {
                int ix = 0;
                for (; (ix < N-head) and begin<end; ++ix, *(data++)=*(begin++)) {
                }
                return ix;
            }

            int write_null_terminate() { *(data++)=0; }
            int write_comma() { *(data++)=';'; }
            int write_new_line() { *(data++)='\n'; }
            int write_space() { *(data++)=' '; }
            int write_under_score() { *(data++)='_'; }


        private:
            static uint32_t digits10(uint64_t v) {
                uint32_t result = 1;
                for (;; v /= 10000U, result+=4) {
                    if (v < 10) return result;
                    if (v < 100) return result + 1;
                    if (v < 1000) return result + 2;
                    if (v < 10000) return result + 3;
                }
            }

            static size_t facebook_uint32_to_str(uint32_t value, char *dst)
            {
                static const char digits[201] =
                        "0001020304050607080910111213141516171819"
                        "2021222324252627282930313233343536373839"
                        "4041424344454647484950515253545556575859"
                        "6061626364656667686970717273747576777879"
                        "8081828384858687888990919293949596979899";
                size_t const length = digits10(value);
                size_t next = length - 1;
                while (value >= 100) {
                    auto const i = (value % 100) * 2;
                    value /= 100;
                    dst[next] = digits[i + 1];
                    dst[next - 1] = digits[i];
                    next -= 2;
                }
                // Handle last 1-2 digits
                if (value < 10) {
                    dst[next] = '0' + uint32_t(value);
                } else {
                    auto i = uint32_t(value) * 2;
                    dst[next] = digits[i + 1];
                    dst[next - 1] = digits[i];
                }
                return length;
            }

        };

        static unsigned _internal_composite_v2(sampler_t * sampler,
                                               const GLchar ** sources,
                                               GLint * lengths,
                                               linear_buffer<4000> & storage) {
            const GLchar ** start = sources;
            const auto sub_samplers_count = sampler->sub_samplers_count();
            for (int ix = 0; ix < sub_samplers_count; ++ix) {
                auto how_much = _internal_composite_v2(
                        sampler->sub_sampler(ix), sources, lengths, storage);
                sources+=how_much; lengths+=how_much;
            }

            // uniform struct DATA_ID { float a;  vec2 b; } data_ID;
            const auto id = sampler->id;
            *(sources++) = struct_0;
            *(lengths++) = -1; // null terminated
            // _ID
            auto * id_str = storage;
            auto id_str_len = facebook_uint32_to_str(id, storage);
            *(sources++) = id_str;
            *(lengths++) = id_str_len;
            // uniforms of sampler
            *(sources++) = sampler->uniforms();
            *(lengths++) = -1; // null-terminated but also has \n. might be buggy
            // _ID
            *(sources++) = id_str; *(lengths++) = id_str_len;
            // ;
            *(sources++) = &comma; *(lengths++) = 1;

            // vec4 sampler_ID
            *(sources++) = sampler_pre; *(lengths++) = -1;
            // ID
            *(sources++) = sampler_pre; *(lengths++) = id_str_len;

            // now starts function body of sampler
            int indices[sub_samplers_count + 1]; // sub samplers and data object
            for (int ix = 0; ix < sub_samplers_count + 1; ++ix) {
                indices[ix]=0;
            }



            return sources-start;
        }

        static int index_of_in(const char * a, const char * b, int max_length) {
            for (int ix=0; *b!='\0' and ix<max_length; ++b, ++ix) {
                const bool is_ = is_equal(a, b+ix, max_length);
                if(is_) return ix;
            }
            return -1;
        }

        static bool is_equal(const char * a, const char * b, int max_length) {
            for (; *a==*b and *a!='\0' and *b!='\0' and max_length; ++a, ++b, --max_length) {}
            if(*a=='\0' or max_length==0) return true;
            return false;
        }

        static main_shader_program composite_main_program_from_sampler_v2(sampler_t & sampler) {
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
                                          sampler_main,
                                          main_shader_program::frag_main};
            auto fragment = shader::from_fragment(sources, 7, nullptr);
            prog.attach_shaders(nitrogl::traits::move(vertex),
                                nitrogl::traits::move(fragment));
            prog.resolve_vertex_attributes_and_uniforms_and_link();
            // sampler_t can now cache uniforms variables
            sampler.on_cache_uniforms_locations(prog.id());
            GLchar source[10000];
            prog.fragment().get_source(source, sizeof (source));

            std::cout<<source<<std::endl;
            return prog;
        }

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