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

#include "matrix.h"
#include "../math.h"

namespace nitrogl {

    template<typename number, bool column_major=true>
    class mat3 : public matrix<number, 3, 3, column_major> {
    private:
        using base__ = matrix<number, 3, 3, column_major>;

    public:
        // in this derived class I overload * operator, this will default in
        // c++ to hiding all previous * overloading, so we have to re-expose it
        using base__::operator*;
        using value_type = number;
        using index = unsigned;
        using type_ref = number &;
        using const_type_ref = const number &;
        using matrix_ref = mat3<number> &;
        using const_matrix_ref = const mat3<number> &;
//        using vertex = nitrogl::vertex2<number>;

        static mat3 identity() { return mat3(); }

        static mat3 translate(const_type_ref tx, const_type_ref ty) {
            mat3 mat;
            mat(0, 2) = tx;
            mat(1, 2) = ty;
            return mat;
        }

        static mat3 scale(const_type_ref sx, const_type_ref sy) {
            mat3 mat;
            mat(0, 0) = sx;
            mat(1, 1) = sy;
            return mat;
        }

        static mat3 reflect(bool x_axis, bool y_axis) {
            return scale(y_axis ? -1 : 1, x_axis ? -1 : 1);
        }

        static mat3 shear_x(const_type_ref angles) {
            mat3 mat{};
            mat(0, 1) = nitrogl::math::tan(angles);
            return mat;
        }

        static mat3 shear_y(const_type_ref angles) {
            mat3 mat{};
            mat(1, 0) = nitrogl::math::tan(angles);
            return mat;
        }

        static mat3 rotation(const_type_ref angle) {
            mat3 mat;
            const_type_ref cos_ = nitrogl::math::cos(angle);
            const_type_ref sin_ = nitrogl::math::sin(angle);
            mat(0,0) = cos_;
            mat(0,1) = -sin_;
            mat(1,0) = sin_;
            mat(1,1) = cos_;
            return mat;
        }

        static mat3 rotation(const_type_ref angle,
                            const_type_ref px,
                            const_type_ref py) {
            mat3 mat;
            const_type_ref cos_ = nitrogl::math::cos(angle);
            const_type_ref sin_ = nitrogl::math::sin(angle);

            mat(0,0) = cos_;
            mat(0,1) = -sin_;
            mat(0,2) = -cos_*px + sin_*py + px;

            mat(1,0) = sin_;
            mat(1,1) = cos_;
            mat(1,2) = -sin_*px - cos_*py + py;

            mat(2,0) = 0;
            mat(2,1) = 0;
            mat(2,2) = number(1);

            return mat;
        }

        static mat3 rotation(const_type_ref angle,
                            const_type_ref px,
                            const_type_ref py,
                            const_type_ref sx,
                            const_type_ref sy) {
            mat3 mat;
            const_type_ref cos_ = nitrogl::math::cos(angle);
            const_type_ref sin_ = nitrogl::math::sin(angle);

            mat(0,0) = sx*cos_;
            mat(0,1) = -sy*sin_;
            mat(0,2) = -sx*cos_*px + sy*sin_*py + px;

            mat(1,0) = sx*sin_;
            mat(1,1) = sy*cos_;
            mat(1,2) = -sx*sin_*px - sy*cos_*py + py;

            mat(2,0) = 0;
            mat(2,1) = 0;
            mat(2,2) = number(1);

            return mat;
        }

        mat3() { identity_fill(); };

        template<class Iterable>
        mat3(const Iterable & list) : base__{list} {}
        mat3(const_type_ref fill_value) : base__(fill_value) {}
        mat3(const base__ & mat) : base__(mat) {}
        template<typename T2>
        mat3(const matrix<T2, 3, 3, column_major> & mat) : base__(mat) {}
        virtual ~mat3() = default;

        mat3 operator*(const mat3 &  value) const {
            return base__::operator*(value);
        };

        template<class vertex>
        vertex operator*(const vertex & point) const {
            vertex res;
            const auto & m = (*this);
            constexpr bool c = column_major;
            res.x = m[0]*point.x + m[c?3:1]*point.y + m[c?6:2];
            res.y = m[c?1:3]*point.x + m[4]*point.y + m[c?7:5];
            return res;
        }

        void fill_diagonal(const_type_ref value) {
            index next = 0;
            for (index row = 0; row < this->rows(); ++row, next+=3)
                this->_data[next++] = value;
        }

        matrix_ref identity_fill() {
            this->fill(0);
            number fill_one{1};
            fill_diagonal(fill_one);
            return *this;
        }

        bool isIdentity() const {
            number zero=number{0}, one{1};
            return (
                    this->_data[0]==one  && this->_data[1]==zero && this->_data[2]==zero &&
                    this->_data[3]==zero && this->_data[4]==one  && this->_data[5]==zero &&
                    this->_data[6]==zero && this->_data[7]==zero && this->_data[8]==one);
        }

        // optimized pre/post multiplications
        // pre is applying the transformation to the right, before the current one
        // post is to the left, after the current one
        template<class vertex> mat3 & pre_translate(const vertex & v) {
            auto & m = (*this);
            constexpr bool c = column_major;
            // last column
            m[c?6:2] = m[0]*v.x + m[c?3:1]*v.y + m[c?6:2];
            m[c?7:5] = m[c?1:3]*v.x + m[4]*v.y + m[c?7:5];
            m[8] = m[c?2:6]*v.x + m[c?5:7]*v.y + m[8];
            return *this;
        }
        template<class vertex> mat3 & post_translate(const vertex & v) {
            auto & m = (*this);
            constexpr bool c = column_major;
            m[0]+=v.x*m[c?2:6]; m[c?3:1]+=v.x*m[c?5:7]; m[c?6:2]+=v.x*m[8]; // 1st row
            m[c?1:3]+=v.y*m[c?2:6]; m[4]+=v.y*m[c?5:7]; m[c?7:5]+=v.y*m[8]; // 2nd row
            return *this;
        }

        template<class vertex> mat3 & pre_scale(const vertex & v) {
            auto & m = (*this);
            constexpr bool c = column_major;
//            m[0]*=v.x; m[c?1:3]*=v.x; m[c?2:6]*=v.x; // 1st column
//            m[c?3:1]*=v.y; m[4]*=v.y; m[c?5:7]*=v.y; // 2nd column

            m[0]*=v.x; m[c?1:3]*=v.x; m[c?2:6]*=v.x; // 1st column
            m[c?3:1]*=v.y; m[4]*=v.y; m[c?5:7]*=v.y; // 2nd column
            return *this;
        }
        template<class vertex> mat3 & post_scale(const vertex & v) {
            auto & m = (*this);
            constexpr bool c = column_major;
            m[0]*=v.x; m[c?3:1]*=v.x; m[c?6:2]*=v.x; // 1st row
            m[c?1:3]*=v.y; m[4]*=v.y; m[c?7:5]*=v.y; // 2nd row
            return *this;
        }

    };

    using mat3f = mat3<float>;
}
