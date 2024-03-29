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

#include "../compositing/blend_modes.h"
#include "../compositing/porter_duff.h"
#include "../_internal/main_shader_program.h"
#include "../_internal/string_utils.h"
#include "../samplers/sampler.h"

namespace nitrogl {

    class shader_compositor {
    public:
        shader_compositor()=delete;
        shader_compositor & operator=(const shader_compositor &)=delete;
        shader_compositor operator=(shader_compositor &&)=delete;
        ~shader_compositor()=delete;

        const char * const * db() {
            const static char * _db[3] = { "_0", "_1", "_2"};
            return _db;
        }

        /**
         * sources buffer to pointer for stitching pre defined strings, also
         * includes optional storage for char arrays
         * @tparam N size of sources pointers array
         * @tparam M size of optional storage
         */
        template<unsigned N, unsigned M=1>
        struct sources_buffer {
            static constexpr const GLchar * const comma_and_new_line = ";\n";
            static constexpr const GLchar * const comma_and_2_new_line = ";\n\n";
            static constexpr const GLchar * comma = ";";
            static constexpr const GLchar * lcp = "{";
            static constexpr const GLchar * rcp = "}";
            static constexpr const GLchar * char_comma = ";";
            static constexpr const GLchar * char_space = " ";
            static constexpr const GLchar * char_under_score = "_";
            static constexpr const GLchar * char_null_term = "\0";
            static constexpr const GLchar * char_new_line = "\n";
            const char * sources[N]; // array with pointers to const chars
            int lengths[N];
            char extra_storage[M]; // writable area
            const char* * head_sources; // pointer to array with pointers to const chars
            char * head_storage;
            int * head_lengths;

            sources_buffer() : head_sources(sources), head_storage(extra_storage),
                            head_lengths(lengths), sources{nullptr}, lengths{0},
                            extra_storage{0} {
            }

            void reset() { head_sources=sources; head_storage=extra_storage; head_lengths=lengths; }
            unsigned size() const { return head_sources-sources; }
            unsigned len_sources() const { return head_sources-sources; }
            unsigned len_lengths() const { return head_lengths-lengths; }
            unsigned len_storage() const { return head_storage-extra_storage; }
            struct write_storage_info_t { char * ptr; int len; };

            void write_char_array_pointer(const char * arr, int len=-1) {
                // write a pointer(a bit dangerous), len=-1 means it is null-terminated
                *(head_sources++)=arr;
                *(head_lengths++)=len;
            }
            void write_range_pointer(const char * begin, const char * end) {
                // write a pointer(a bit dangerous), len=-1 means it is null-terminated
                *(head_sources++)=begin;
                *(head_lengths++)=end-begin;
            }

            write_storage_info_t write_int(int v) {
                // converts int to char array in storage and then copies the pointer
                *(head_sources++)=head_storage;
                unsigned len = nitrogl::facebook_uint32_to_str(v, head_storage);
                head_storage+=len;
                *(head_lengths++)=len;
                return { head_storage-len, int(len) };
            }
            write_storage_info_t write_int_to_storage(int v) {
                // converts int to char array in storage and then copies the pointer
                unsigned len = nitrogl::facebook_uint32_to_str(v, head_storage);
                head_storage+=len;
                return { head_storage-len, int(len) };
            }
            write_storage_info_t write_char_array_to_storage(char * arr) {
                // copy null-terminated array into storage and then write to sources
                unsigned ix = 0;
                const auto available = N-len_storage();
                for (; (ix < available) && *arr; ++ix, *(head_storage++)=*(arr++)) {
                }
                write_char_array_pointer(head_storage-ix, ix);
                return {head_storage-ix, ix};
            }
            write_storage_info_t write_range_to_storage(const char * begin, const char * end) {
                // range does not contain null-term
                unsigned ix = 0;
                const auto available = N-len_storage();
                for (; (ix < available) && begin<end; ++ix, *(head_storage++)=*(begin++)) {
                }
                write_char_array_pointer(head_storage-ix, ix);
                return {head_storage-ix, ix};
            }
            write_storage_info_t write_char_to_storage_and_buffers(char v) {
                *(head_storage++) = v;
                *(head_sources++) = head_storage-1;
                *(lengths++) = 1;
                return { head_storage-1, 1 };
            }

            void write_null_terminate() { write_char_array_pointer(char_null_term, 1); }
            void write_comma() { write_char_array_pointer(char_comma, 1); }
            void write_new_line() { write_char_array_pointer(char_new_line, 1); }
            void write_space() { write_char_array_pointer(char_space, 1); }
            void write_under_score() { write_char_array_pointer(char_under_score, 1); }
            void write_lcp() { write_char_array_pointer(lcp, 1); }
            void write_rcp() { write_char_array_pointer(rcp, 1); }
            void write_comma_and_newline() { write_char_array_pointer(comma_and_new_line, 2); }
            void write_comma_and_2_newline() { write_char_array_pointer(comma_and_2_new_line, 3); }
        };

        template<class number> static number min(number a, number b) { return a<b?a:b;}
        template<class number> static number max(number a, number b) { return a<b?b:a;}

    private:
        template<unsigned N, unsigned M>
        static void _internal_composite(sampler_t * sampler, sources_buffer<N, M> & buffer) {
            // if the sampler is nullptr or was already visited, then we don't need to write it
            if(sampler==nullptr || sampler->traversal_info().visited) return;
            // otherwise, recurse bottom-up
            const auto sub_samplers_count = sampler->sub_samplers_count();
            for (int ix = 0; ix < sub_samplers_count; ++ix)
                _internal_composite(sampler->sub_sampler(ix), buffer);

            sampler->traversal_info().visited=true;

            // uniform struct DATA_ID { float a;  vec2 b; } data_ID;
            const bool has_uniforms_data = !nitrogl::is_empty(sampler->uniforms());
            if(has_uniforms_data) {
                buffer.write_char_array_pointer("uniform struct DATA_", -1);
                buffer.write_char_array_pointer(sampler->traversal_info().id_str(),
                                                sampler->traversal_info().size_id_str()); // ID from previous stored value
                buffer.write_char_array_pointer(sampler->uniforms(), -1);
                buffer.write_char_array_pointer("data_", -1);
                buffer.write_char_array_pointer(sampler->traversal_info().id_str(),
                                                sampler->traversal_info().size_id_str()); // ID from previous stored value
                buffer.write_comma_and_2_newline();
            }

            // vec4 sampler_ID
            buffer.write_char_array_pointer("vec4 sampler_", -1);
            buffer.write_char_array_pointer(sampler->traversal_info().id_str(),
                                            sampler->traversal_info().size_id_str());

            // now the tough part starts function body of sampler. Our goal
            // is to track 'data.' and 'sampler_' strings and to stitch:
            // data. --> data_{SAMPLER_ID}
            // sampler_ --> data_{SAMPLER_ID}
            // strategy is to make them race for the next one
            const auto * main = nitrogl::find_first_not_of_in(sampler->main(), '\n', -1);
            const auto handle = [&](race_t & race, const char * from) -> const char * {
                race.handled=true;
                switch (race.id) {
                    case 0: { // sampler_{sub_sampler_local_id} -> sampler_{sub_sampler_global_id}
                        auto begin_0 = from; // latest end loose
                        auto end_0 = race.next + race.name_len; // end of sampler_
                        buffer.write_range_pointer(begin_0, end_0); // stitch [main, sampler_)
                        // parse local_id
                        auto begin_1 = nitrogl::index_of_in("(", end_0, 1); // ptr to (
                        auto local_id = nitrogl::s2i(end_0, begin_1-end_0); // local_id to int
                        // stitch {global_id}
                        buffer.write_char_array_pointer(sampler->sub_sampler(local_id)->traversal_info().id_str(),
                                                        sampler->sub_sampler(local_id)->traversal_info().size_id_str());
                        race.handled=true;
                        return begin_1;
                    }
                    case 1: { // data. --> data_{current_sampler_global_id}.
                        auto begin_0 = from; // latest end loose
                        auto end_0 = race.next + race.name_len - 1; // end of '...data'
                        buffer.write_range_pointer(begin_0, end_0); // stitch [main, data)
                        //
                        buffer.write_under_score(); // stitch _
                        // stitch {current_sampler_id}
                        buffer.write_char_array_pointer(sampler->traversal_info().id_str(),
                                                        sampler->traversal_info().size_id_str());
                        race.handled=true;
                        return end_0;
                    }
                }
                return nullptr;
            };
            race_t races[2] = {
                    {0, "sampler_", 8, main, true, true},
                    {1, "data.", 5, main, true, true},
            };
            const auto * latest_main = main;
            if(sub_samplers_count==0) races[0].enabled=false; // disable 0
            if(!has_uniforms_data) races[1].enabled=false; // disable 1
            int arg_min=0;
            for(; arg_min!=-1; ) {
                arg_min=-1;
                // pick next index of handled and argmin
                for (int ix = 0; ix < 2; ++ix) {
                    auto & race = races[ix];
                    if(!race.enabled) continue;
                    if(race.handled) {
                        race.next = index_of_in(race.name, latest_main, race.name_len);
                        if(race.next==nullptr) { // did not find
                            // let's disable it entirely
                            race.enabled=false;
                            continue;
                        }
                        // flag it is pending a handle
                        race.handled=false;
                    }
                }
                for (int ix = 0; ix < 2; ++ix) {
                    auto & race = races[ix];
                    if(!race.enabled) continue;
                    if(!race.handled) {
                        if(arg_min==-1 ||
                            races[arg_min].next==nullptr ||
                            race.next<races[arg_min].next)
                            arg_min=ix;
                    }
                }
                if(arg_min!=-1)
                    latest_main=handle(races[arg_min], latest_main);
            }
            buffer.write_char_array_pointer(latest_main, -1); // stitch [latest_main, end)
            buffer.write_new_line(); // stitch [latest_main, end)
        }

        struct race_t {
            int id; // this dictates behaviour
            const char * name; int name_len;
            const char * next;
            bool handled;
            bool enabled;
        };

    public:
        static bool composite_main_program_from_sampler(main_shader_program & program,
                                                        sampler_t & sampler,
                                                        const GLchar * glsl_version=nullptr,
                                                        bool is_premul_alpha_result=true,
                                                        const nitrogl::blend_mode_t blend_mode=nullptr,
                                                        const nitrogl::compositor_t compositor=nullptr) {
            // fragment shards
            using buffers_type = sources_buffer<1000, 1>;
            static buffers_type buffers{};
            buffers.reset();
            // write version
            const GLchar * glsl_v = glsl_version ? glsl_version : main_shader_program::glsl_version;
            buffers.write_char_array_pointer(glsl_v);
            buffers.write_new_line();
            // write compatability
            buffers.write_char_array_pointer(main_shader_program::shader_compat);
            // write frag variables
            buffers.write_char_array_pointer(main_shader_program::frag_other);
            buffers.write_char_array_pointer(nitrogl::porter_duff::base());
            // add samplers tree recursively
            _internal_composite(&sampler, buffers);
            // add define (#define __SAMPLER_MAIN sampler_{id})
            buffers.write_char_array_pointer(main_shader_program::define_sampler);
            buffers.write_char_array_pointer(sampler.traversal_info().id_str(),
                                             sampler.traversal_info().size_id_str());
            buffers.write_new_line();
            // write compositing stuff
            if(compositor) buffers.write_char_array_pointer(compositor);
            if(blend_mode) buffers.write_char_array_pointer(blend_mode);
            if(is_premul_alpha_result)
                buffers.write_char_array_pointer(main_shader_program::define_premul_alpha);
            // write main shader
            buffers.write_char_array_pointer(main_shader_program::frag_main);
            //

            auto & vertex = program.vertex();
            auto & fragment = program.fragment();

            // vertex shader is always the same/constant here, so we can save a compilation once it is hot
            // or was used compiled once in the past.
            if(!vertex.isCompiled()) {
                const GLchar * vertex_shader_sources[3] =
                        { main_shader_program::glsl_version, main_shader_program::shader_compat,
                          main_shader_program::vert };
                vertex.updateShaderSource(vertex_shader_sources, 3, nullptr, true);
            }
            bool stat_compile = fragment.updateShaderSource(buffers.sources, buffers.size(),
                                                            buffers.lengths, true);
            if(!stat_compile) {
#ifdef NITROGL_DEBUG_MODE
                GLchar source[10000];
                fragment.info_log(source, sizeof(source));
                std::cout << source << std::endl;
#endif
#ifndef NITROGL_DISABLE_THROW
                struct compile_error{};
                throw compile_error{};
#endif
                return false;
            }
            program.resolve_vertex_attributes_and_uniforms_and_link();
            // sampler can now cache uniforms variables
            sampler.cache_uniforms_locations(program.id());

#ifdef NITROGL_DEBUG_MODE
            GLchar source[10000];
            program.fragment().get_source(source, sizeof (source));
            std::cout<< source <<std::endl;
#endif
            return true;
        }

    };

}