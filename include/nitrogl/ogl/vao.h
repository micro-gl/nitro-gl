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

    class vao_t {
        GLuint _id;

        void generate() { if(!_id) glGenVertexArrays(1, &_id); }

    public:
        vao_t() : _id(0) { generate(); };
        vao_t(vao_t && o)  noexcept : _id(o._id) { o._id=0; }
        vao_t(const vao_t &)=default;
        vao_t & operator=(vao_t && o)  noexcept { _id=o._id; o._id=0; return *this; }
        vao_t & operator=(const vao_t &)=default;
        ~vao_t() { del(); unbind(); }

        bool wasGenerated() const { return _id; }
        GLuint id() const { return _id; }
        void del() { if(_id) { glDeleteVertexArrays(1, &_id); _id=0; } }
        void bind() const { glBindVertexArray(_id); }
        static void unbind() { glBindVertexArray(0); }
    };

}

