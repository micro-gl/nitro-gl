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
        static constexpr const GLchar lcp = '{';
        static constexpr const GLchar rcp = '}';
        static constexpr const GLchar * const sampler_pre = "vec4 sampler_";
        static constexpr const GLchar * const sampler = "sampler_";
        static constexpr const GLchar * const data = "data.";

        template<unsigned N, unsigned M>
        struct sources_buffer {
            static const char char_comma = ';';
            static const char char_space = ' ';
            static const char char_under_score = '_';
            static const char char_null_term = '\0';
            static const char char_new_line = '\n';
            char * const sources[N];
            int const lengths[N];
            char const extra_storage[M];
            char ** head_sources;
            char * head_storage;
            int * head_lengths;

            sources_buffer() : head_sources(sources), head_storage(extra_storage),
                            head_lengths(lengths), sources(nullptr), lengths(-1),
                            extra_storage(0) {
            }

            void reset() { head_sources=sources; head_storage=extra_storage; }
            unsigned len_sources() const { return head_sources-sources; }
            unsigned len_lengths() const { return head_lengths-lengths; }
            unsigned len_storage() const { return head_storage-extra_storage; }
            struct write_storage_info_t { char * ptr; int len; };
            write_storage_info_t write_int(int v) {
                // converts int to char array in storage and then copies the pointer
                *(head_sources++)=head_storage;
                unsigned len = facebook_uint32_to_str(v, head_storage);
                head_storage+=len;
                *(head_lengths++)=len;
                return { head_storage-len, len };
            }

            void write_char_array_pointer(char * arr, int len=-1) {
                // write a pointer(a bit dangerous), len=-1 means it is null-terminated
                *(head_sources++)=arr;
                *(head_lengths++)=len;
            }
            void write_range_pointer(char * begin, const char * end) {
                // write a pointer(a bit dangerous), len=-1 means it is null-terminated
                *(head_sources++)=begin;
                *(head_lengths++)=end-begin;
            }
            write_storage_info_t write_char_array(char * arr) {
                // copy null-terminated array into storage and then write to sources
                unsigned ix = 0;
                const auto available = N-len_storage();
                for (; (ix < available) and *arr; ++ix, *(head_storage++)=*(arr++)) {
                }
                write_char_array_pointer(head_storage-ix, ix);
                return {head_storage-ix, ix};
            }

            write_storage_info_t write_range(const char * begin, const char * end) {
                // range does not contain null-term
                unsigned ix = 0;
                const auto available = N-len_storage();
                for (; (ix < available) and begin<end; ++ix, *(head_storage++)=*(begin++)) {
                }
                write_char_array_pointer(head_storage-ix, ix);
                return {head_storage-ix, ix};
            }

            write_storage_info_t write_char(char v) {
                *(head_storage++) = v;
                *(head_sources++) = head_storage-1;
                *(lengths++) = 1;
                return { head_storage-1, 1 };
            }

            int write_null_terminate() { write_char_array_pointer(&char_null_term, 1); }
            int write_comma() { write_char_array_pointer(&char_comma, 1); }
            int write_new_line() { write_char_array_pointer(&char_new_line, 1); }
            int write_space() { write_char_array_pointer(&char_space, 1); }
            int write_under_score() { write_char_array_pointer(&char_under_score, 1); }
            int write_lcp() { write_char_array_pointer(&lcp, 1); }
            int write_rcp() { write_char_array_pointer(&rcp, 1); }


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

            static size_t facebook_uint32_to_str(uint32_t value, char *dst) {
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

        template<class number> static number min(number a, number b) { return a<b?a:b;}
        template<class number> static number max(number a, number b) { return a<b?b:a;}

        template<unsigned N, unsigned M>
        static void _internal_composite_v2(sampler_t * sampler,
                                               sources_buffer<N, M> & buffer,
                                               typename sources_buffer<N, M>::write_storage_info_t & sampler_string_id_lookup) {
            using wi = typename sources_buffer<N, M>::write_storage_info_t;
            const auto sub_samplers_count = sampler->sub_samplers_count();
            wi sub_sampler_string_ids_lookup[sub_samplers_count];
            for (int ix = 0; ix < sub_samplers_count; ++ix) {
                auto how_much = _internal_composite_v2(
                        sampler->sub_sampler(ix), buffer, sub_sampler_string_ids_lookup+ix);
            }

            // uniform struct DATA_ID { float a;  vec2 b; } data_ID;
            const auto id = sampler->id;
            buffer.write_char_array_pointer("uniform struct DATA_", -1);
            auto info_id = buffer.write_int(id);
            sampler_string_id_lookup = info_id; // record for parent, so he will have quick lookup
            buffer.write_char_array_pointer(sampler->uniforms(), -1);
            buffer.write_char_array_pointer("data_", -1);
            buffer.write_char_array_pointer(info_id.ptr, info_id.len); // ID from previous stored value
            buffer.write_comma();
            buffer.write_new_line();

            // vec4 sampler_ID
            buffer.write_char_array_pointer("vec4 sampler_", -1);
            buffer.write_char_array_pointer(info_id.ptr, info_id.len);

            // now the tough part starts function body of sampler. Our goal
            // is to track 'data.' and 'sampler_' strings and to stitch:
            // data. --> data_{SAMPLER_ID}
            // sampler_ --> data_{SAMPLER_ID}
            // strategy is to make them race for the next one
            const auto * main = sampler->main();
            const auto handle = [&](race_t & race, char * from) -> const char * {
                race.handled=true;
                switch (race.id) {
                    case 0: { // sampler_{sub_sampler_local_id} -> sampler_{sub_sampler_global_id}
                        auto begin_0 = from; // latest end loose
                        auto end_0 = race.next + race.name_len; // end of sampler_
                        buffer.write_range_pointer(begin_0, end_0); // stitch [main, sampler_)
                        // parse local_id
                        auto begin_1 = index_of_in("(", end_0, 1); // ptr to (
                        auto local_id = s2i(end_0, begin_1-end_0); // local_id to int , todo:: remove white space and 005 etc..
                        const auto global_id = sub_sampler_string_ids_lookup[local_id]; // global-id
                        buffer.write_char_array_pointer(global_id.ptr, global_id.len); // stitch {global_id}
                        race.handled=true;
                        return begin_1;
                    }
                    case 1: { // data. --> data_{current_sampler_global_id}
                        auto begin_0 = from; // latest end loose
                        auto end_0 = race.next + race.name_len - 1; // end of '...data'
                        buffer.write_range_pointer(begin_0, end_0); // stitch [main, data)
                        //
                        buffer.write_under_score(); // stitch _
                        buffer.write_char_array_pointer(info_id.ptr, info_id.len); // stitch {current_sampler_id}
                        race.handled=true;
                        return end_0;
                    }
                }
                return nullptr;
            };
            race_t races[2] = {
                    {0, "sampler_", 8, nullptr, true},
                    {1, "data.", 5, nullptr, true},
            };
            const auto * latest_main = main;

            for(int arg_min=0; arg_min!=-1; latest_main=handle(races[arg_min], latest_main), arg_min=-1) {
                // pick next index of handled and argmin
                for (int ix = 0; ix < 2; ++ix) {
                    auto & race = races[ix];
                    if(race.handled) {
                        race.next = index_of_in(race.name, latest_main, race.name_len);
                        if(race.next==nullptr) continue;
                        race.handled=false;
                        if(arg_min==-1 or race.next<races[arg_min].next) arg_min=ix;
                    }
                }
            }
            buffer.write_char_array_pointer(latest_main, -1); // stitch [latest_main, end)
            buffer.write_new_line(); // stitch [latest_main, end)
        }

        static int s2i(const char *c, int len) {
            int s = 1, res = 0;
            if (c[0] == '-') { s = -1; ++c; }
            for (; len and *c ; --len, ++c) {
                res = res*10 + (*c - '0');
            }
            return res*s;
        }

        struct race_t {
            int id; // this dictates behaviour
            const char * name; int name_len;
            const char * next;
            bool handled;
        };

        static int handled_arg_min(race_t * races, const int length) {
            // find first non handled
            int arg=-1;
            for (int ix = 0; ix < length; ++ix) {
                if(races[ix].handled) arg=ix;
            }
            if(arg==-1) return -1;

            for (int ix = arg; ix < length; ++ix) {
                if(races[ix].next && races[ix].handled &&
                   (races[ix].next < races[arg].next) )
                    arg=ix;
            }
            return arg;
        }

        static const char * index_of_in(const char * a, const char * b, int max_length_of_a) {
            for (int ix=0; *b and ix<max_length_of_a; ++b, ++ix) {
                const bool is_ = is_equal(a, b+ix, max_length_of_a);
                if(is_) return (b+ix);
            }
            return  nullptr;
        }

        static bool is_equal(const char * a, const char * b, int max_length) {
            for (; *a==*b and *a and *b and max_length; ++a, ++b, --max_length) {}
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