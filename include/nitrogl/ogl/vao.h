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

#ifdef SUPPORTS_VAO
    class vao_t {
        GLuint _id;
        bool owner;

        void generate() { if(!_id) glGenVertexArrays(1, &_id); }
        vao_t(GLuint id, bool owner) : _id(id), owner(owner) {};

    public:
        static vao_t from_id(GLuint id, bool owner=true) { return { id, owner }; }
        vao_t() : _id(0), owner(true) { generate(); };
        vao_t(vao_t && o)  noexcept : _id(o._id), owner(o.owner) { o.owner=false; }
        vao_t(const vao_t & o) : _id(o._id), owner(false) {}
        vao_t & operator=(const vao_t & o) {
            if(&o!=this) { del(); _id=o._id; owner=false; }
            return *this;
        };
        vao_t & operator=(vao_t && o) noexcept {
            if(&o!=this) { del(); _id=o._id; owner=o.owner; o.owner=false; }
            return *this;
        }
        ~vao_t() { del(); unbind(); }

        bool wasGenerated() const { return _id; }
        GLuint id() const { return _id; }
        void del() { if(_id && owner) { glDeleteVertexArrays(1, &_id); _id=0; } }
        void bind() const { glBindVertexArray(_id); }
        static void unbind() { glBindVertexArray(0); }
    };
#else
    class vao_t {
    public:
        static vao_t from_id(GLuint id, bool owner=true) { return {}; }
        vao_t()=default;
        ~vao_t()=default;
        bool wasGenerated() const { return false; }
        GLuint id() const { return 0; }
        void del() {}
        void bind() const {}
        static void unbind() {}
    };
#endif
}

