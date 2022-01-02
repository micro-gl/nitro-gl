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

#include "gl_texture.h"

namespace nitrogl {

    class fbo_t {
        GLuint _id;
        bool owner;

        explicit fbo_t(GLint id) : _id(id), owner(false) {};

    public:
        static fbo_t un_generated() { return fbo_t(0); }
        static fbo_t from_current() {
            GLint id=0;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &id);
            return fbo_t(id);
        }
        fbo_t() : _id(0), owner(true) { generate(); };
        fbo_t(fbo_t && o)  noexcept : _id(o._id), owner(o.owner) { o.owner=false; }
        fbo_t(const fbo_t & o) : _id(o._id), owner(false) {}
        fbo_t & operator=(const fbo_t & o) {
            if(&o!=this) { del(); _id=o._id; owner=false; }
            return *this;
        };
        fbo_t & operator=(fbo_t && o) noexcept {
            if(&o!=this) { del(); _id=o._id; owner=o.owner; o.owner=false; }
            return *this;
        }
        ~fbo_t() { del(); unbind(); }

    public:

        void generate() { if(!_id) glGenFramebuffers(1, &_id); }
        void attachTexture(const gl_texture & texture) const {
            bind();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, texture.id(), 0);
            // check for completeness
//            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//                log("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
        }
        bool wasGenerated() const { return _id; }
        GLuint id() const { return _id; }
        void del() { if(_id && owner) { glDeleteFramebuffers(1, &_id); _id=0; owner=false; } }
        void bind() const { glBindFramebuffer(GL_FRAMEBUFFER, _id); }
        void bind_read() const { glBindFramebuffer(GL_READ_FRAMEBUFFER, _id); }
        void bind_draw() const { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _id); }
        static void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
    };

}

