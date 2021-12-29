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

    class vao {
    public:
        vao() : _id(0) { glGenVertexArrays(1, &_id); };
        ~vao() { del(); unbind(); }
        GLuint id() const { return _id; }
        void del() { if(_id) { glDeleteVertexArrays(1, &_id); _id=0; } }
        void bind() const { glBindVertexArray(_id); }
        static void unbind() { glBindVertexArray(0); }

    private:
        GLuint _id;
    };

}

