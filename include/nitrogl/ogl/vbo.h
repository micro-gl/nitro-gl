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

namespace nitrogl {

    class vbo_t {
        GLuint _id;
        bool owner;

        void generate() { if(!_id) glGenBuffers(1, &_id); }
        vbo_t(GLuint id, bool owner) : _id(id), owner(owner) {};

    public:
        static vbo_t from_id(GLuint id, bool owner=true) { return { id, owner }; }
        vbo_t() : _id(0), owner(true) { generate(); };
        vbo_t(vbo_t && o)  noexcept : _id(o._id), owner(o.owner) { o.owner=false; }
        vbo_t(const vbo_t & o) : _id(o._id), owner(false) {}
        vbo_t & operator=(const vbo_t & o) {
            if(&o!=this) { del(); _id=o._id; owner=false; }
            return *this;
        };
        vbo_t & operator=(vbo_t && o) noexcept {
            if(&o!=this) { del(); _id=o._id; owner=o.owner; o.owner=false; }
            return *this;
        }
        ~vbo_t() { del(); unbind(); }

        bool wasGenerated() const { return _id; }
        void uploadData(const void * array, GLsizeiptr array_size_bytes, GLenum usage=GL_STATIC_DRAW) const {
            if(_id==0) return;
            bind();
            glBufferData(GL_ARRAY_BUFFER, array_size_bytes, array, usage);
        }
        void uploadSubData(GLintptr offset, const void *array, GLuint size_bytes) const {
            if(_id==0) return;
            bind();
            glBufferSubData(GL_ARRAY_BUFFER, offset, size_bytes, array);
        }
        GLuint id() const { return _id; }
        void del() { if(_id && owner) { glDeleteBuffers(1, &_id); _id=0; } }
        void bind() const { glBindBuffer(GL_ARRAY_BUFFER, _id); }
        static void unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
    };

}

