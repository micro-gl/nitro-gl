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

#include "shader.h"
#include "../traits.h"

namespace nitrogl {
//#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define OFFSET(by) (reinterpret_cast<void*>((by)))

    class shader_program {
    private:
        shader _vertex, _fragment;
        GLuint _id;
        GLint _last_link_status=GL_FALSE;
        bool owner;

    public:
        // attributes are vec/1/2/3/4, they are vector made of elements of a single type.
        // integer correspond to ivecn/uvecn/bvecn. Float to vecn. Double to dvecn
        enum class shader_attribute_component_type { Float, Integer, Double };
        /**
         * NOTES about generic attribs to locations:
         * 1. After linking a program, every vertex attrib location is defined in the vertex
         *    shader. only relinking can change that.
         * 2. you can query the location by name after program linking of attributes and then point the arrays.
         * 3. you can decide the locations by name with glBindAttribLocation, but you will have to relink
         * 4. you can decide the locations inside the vertex shader with (location=#) specifier
         *
         * a bit of defs:
         * 1. generic vertex attribute = spread out vec1, vec2, vec3 or vec4
         * 2. each vec is made up of components
         * 3. interleaved examples = [(x,y,z),(u,v), (x,y,z),(u,v), (x,y,z),(u,v), ...]
         * 4. non-interleaved examples = [(x,y,z),(x,y,z),(x,y,z), (u,v),(u,v),(u,v), ...]
         * 5. stride is constant for interleaved = sizeof((x,y,z))+sizeof((u,v)) = 5
         * 6. stride for non-interleaved is per vertex attribute. stride((x,y,z))=sizeof((x,y,z))=3, stride((u,v))=2
         *
         */
        struct vertex_attr_t {
            const GLchar * name; // name of vertex attribute
            GLint location; // the index of the attribute in the vertex shader
            // the type of components in vertex attribute in shader.
            // vec3->float. ivec2->integer etc...
            shader_attribute_component_type shader_component_type;
        };

        struct uniform_t {
            const GLchar * name=nullptr; // name of uniform
            GLint location=0; // the location
        };

        static shader_program from_shaders(const shader & vertex, const shader & fragment) {
            shader_program prog;
            prog.attach_shaders(vertex, fragment);
            prog.link();
            return prog;
        }
        // ctor: init with empty shaders
        shader_program() : _vertex(shader::null_shader), _fragment(shader::null_shader),
                owner(true), _id(0), _last_link_status(GL_FALSE) {
            create();
        }
        shader_program(const shader & vertex, const shader & fragment) :
                _vertex(vertex), _fragment(fragment), owner(true), _id(0), _last_link_status(GL_FALSE) {
            create();
            glAttachShader(_id, _vertex.id());
            glAttachShader(_id, _fragment.id());
            link();
        }
        shader_program(shader && vertex, shader && fragment) :
                    _vertex(nitrogl::traits::move(vertex)), _fragment(nitrogl::traits::move(fragment)),
                    owner(true), _id(0), _last_link_status(GL_FALSE) {
            create();
            glAttachShader(_id, _vertex.id());
            glAttachShader(_id, _fragment.id());
            link();
        }
        shader_program(shader_program && o) noexcept : _id(o._id), _last_link_status(o._last_link_status),
                        _vertex(nitrogl::traits::move(o._vertex)),
                        _fragment(nitrogl::traits::move(o._fragment)), owner(o.owner) {
            // we don't move vertex and fragment shades, we dont want to own them.
            o.owner=false;
        }
        shader_program & operator=(shader_program && o) noexcept {
            if(this!=&o) {
                _id=o._id; _last_link_status=o._last_link_status;
                _vertex=nitrogl::traits::move(o._vertex);
                _fragment=nitrogl::traits::move(o._fragment);
                owner=o.owner; o.owner=false;
            }
            return *this;
        }
        shader_program(const shader_program & o) : _id(o._id), _last_link_status(o._last_link_status),
            _vertex(o._vertex), _fragment(o._fragment), owner(false) {}
        shader_program& operator=(const shader_program & o) {
            if(this!=&o) {
                _id=o._id; _vertex=o._vertex; _fragment=o._fragment;
                owner=false;
            }
            return *this;
        }
        ~shader_program() { del(); unuse(); }

        bool wasCreated() const { return _id; }
        void create() { if(!_id) _id = glCreateProgram(); }
        void attach_shaders(const shader & vertex, const shader & fragment) {
            _vertex = vertex;
            _fragment = fragment;
            glAttachShader(_id, _vertex.id());
            glAttachShader(_id, _fragment.id());
        }
        void attach_shaders(shader && vertex, shader && fragment) {
            _vertex = nitrogl::traits::move(vertex);
            _fragment = nitrogl::traits::move(fragment);
            glAttachShader(_id, _vertex.id());
            glAttachShader(_id, _fragment.id());
        }
        void detachShaders() const {
            if(_vertex.id()) glDetachShader(_id, _vertex.id());
            if(_fragment.id()) glDetachShader(_id, _fragment.id());
        }
        GLuint id() const { return _id; }
        shader & vertex() { return _vertex; }
        shader & fragment() { return _fragment; }
        void use() const { glUseProgram(_id); }
        static void unuse() { glUseProgram(0); }
        bool link() {
            glLinkProgram(_id);
            // it is okay to have a get after a gl command
            glGetProgramiv(_id, GL_LINK_STATUS, &_last_link_status);
            return _last_link_status;
        }

        GLint info_log(char * log_buffer = nullptr, GLint log_buffer_size=0) const {
            if(!log_buffer or !glIsProgram(_id)) return 0;
            // Shader copied log length
            GLint copied_length = 0;
            // copy info log
            glGetProgramInfoLog(_id, log_buffer_size, &copied_length, log_buffer);
            return copied_length;
        }
        void del() {
            if(!(_id && owner)) return;
            detachShaders();
            glDeleteProgram(_id);
            _id=0;
        }

        // generic attributes
        //
        bool wasLastLinkSuccessful() const {
            return _last_link_status;
//            GLint stat; glGetProgramiv(_id, GL_LINK_STATUS, &stat); return stat;
        }
        void bindAttribLocation(GLuint index, const GLchar *name) const {
            glBindAttribLocation(_id, index, name);
            // you have to link again
        }
        GLint attributeLocationByName(const GLchar * name) const {
            return glGetAttribLocation(_id, name);
        }

        struct vbo_attr_t {
            // the type of element in VBO, this is important because opengl will know
            // better how to convert it to the vertex shader processor
            GLenum type;
            // the type of components in vertex attribute in shader.
            // vec3->float. ivec2->integer etc...
            GLuint size; // the number of components in attribute array vbo (1,2,3,4)
            // the attribute's first relative occurrence offset in the VBO
            const void * offset;
            // stride can be calculated automatically if the buffer is interleaved or non.
            GLsizei stride;
            GLuint vbo; // corresponding vbo
        };

        void enableLocations(const vertex_attr_t * attrs, unsigned length) const {
            // if you have VAO support, then this is part of VAO state
            for (; length!=0 ; --length, ++attrs) glEnableVertexAttribArray(attrs->location);
        };
        void disableLocations(const vertex_attr_t * attrs, unsigned length) const {
            // if you have VAO support, then don't run this method. Otherwise, do.
            for (; length!=0 ; --length, ++attrs) glDisableVertexAttribArray(attrs->location);
        };

        /**
         * Helper method, this should run only once, depends only on shader
         * three modes:
         * 1. { location<0, name!=nullptr } --> first link if hasn't, then query glGetAttribLocation
         * 1. { location>=0, name!=nullptr } --> first glBindAttribLocation, and re-link at end so it will affect shader
         * 1. { location>=0, name==nullptr } --> we assume user used (location=#) specifier in shader, nothing to do
         * 1. { location<0, name==nullptr } --> error
         * @param attrs
         * @param length
         * @return
         */
        bool setOrGetVertexAttributesLocations(vertex_attr_t * attrs, unsigned length) {
            // let's deal with requested explicit locations bindings.
            bool binding_occured=false;
            for (auto * it = attrs; it < attrs + length; ++it) {
                if(it->location<0) continue;
                glBindAttribLocation(_id, it->location, it->name);
                binding_occured = true;
            }
            // we must relink to make bind take effect, bind can happen only before linking does.
            if(binding_occured) {
                link();
                if(!wasLastLinkSuccessful()) return false;
            }

            // let's deal with automatic locations. program has to be linked
            if(!wasLastLinkSuccessful()) link(); // in case no binding was requested

            for (auto * it = attrs; it < attrs + length; ++it) {
                if(it->location>=0) continue;
                if(it->name==nullptr) return false; // must have name
                it->location = glGetAttribLocation(_id, it->name);
            }
            return true;
        };

        bool setUniformLocations(uniform_t * attrs, unsigned length) {
            if(!wasLastLinkSuccessful()) link(); // in case no binding was requested
            for (auto * it = attrs; it < attrs + length; ++it) {
                attrs->location= uniformLocationByName(attrs->name);
            }
            return true;
        };

        /**
         * Helper method, connect VBOs to vertex Shader Attributes.
         * NOTES:
         * 1. If supports VAO, then
         *    - If VBO is PREDICTABLE==VBO (constant stride and offset for each vertex attribute in vbo), then:
         *      THIS METHOD SHOULD RUN ONCE AT INIT WHILE VAO IS BOUND. THEN REUSE VAO BY BINDING AT DRAW PHASE.
         *    - If VBO is NOT PREDICTABLE (non-interleaved multiple attributes, that change in size), THIS SHOULD RUN EVERYTIME things change
         * 2. If no VAO support, then ALWAYS RUN this method before draw.
         * @param attrs
         * @param length
         * @return
         */
        bool pointVertexAtrributes(const vbo_attr_t * attrs_vbo, const vertex_attr_t * attrs_vertex, unsigned length) const {
            bool uniform_vbo=true;
            { // for optimization, inspect if they use same VBO
                const auto vbo=attrs_vbo->vbo;
                for (auto * it = attrs_vbo; it < attrs_vbo + length; ++it) {
                    if(vbo!=it->vbo) { uniform_vbo=false; break; }
                }
            }
            // this avoids extra bindings if all the vertex attributes are mapped
            // from the same vbo
            if(uniform_vbo && attrs_vbo->vbo>=0) glBindBuffer(GL_ARRAY_BUFFER, attrs_vbo->vbo);

            auto * it2 = attrs_vertex;
            for (auto * it = attrs_vbo; it < attrs_vbo + length; ++it, ++it2) {
                // so we have to bind the vbo where the vertex attributes will be at.
                // then enable the location and then point the shader program. If using VAO,
                // then those are part of its state (VBO binding is not, only the mapping from VBO
                // to the vertex shader)
                if(!uniform_vbo && it->vbo>=0) glBindBuffer(GL_ARRAY_BUFFER, it->vbo);
                // enable generic vertex attrib for bound VBO
                glEnableVertexAttribArray((GLuint)it2->location);
                switch (it2->shader_component_type) {
                    case shader_attribute_component_type::Float:
                        glVertexAttribPointer((GLuint)it2->location, GLint(it->size), it->type,
                                               GL_FALSE, it->stride, it->offset);
                        break;
#ifdef SUPPORTS_INT_ATTRIBUTES
                    case shader_attribute_component_type::Integer:
                        glVertexAttribIPointer((GLuint)it->location, GLint(it->size), it->type,
                                               it->stride, it->offset);
                        break;
#endif
#ifdef SUPPORTS_LONG_ATTRIBUTES
                    case shader_attribute_component_type::Double:
                        glVertexAttribLPointer((GLuint)it->location, GLint(it->size), it->type,
                                               it->stride, it->offset);
                        break;
#endif
                    default: return false;
                        break;
                }
            }

            return true;
        }

        // uniforms
        GLint uniformLocationByName(const GLchar * name) const {
            return glGetUniformLocation(_id, name);
        }

    };

}