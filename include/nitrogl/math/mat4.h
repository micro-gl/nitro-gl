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
#include "mat3.h"
#include "../math.h"
#include "vertex4.h"

namespace nitrogl {

    template<typename number, bool column_major=true>
    class mat4 : public matrix<number, 4, 4, column_major> {
    private:
        using base__ = matrix<number, 4, 4, column_major>;

    public:
        // in this derived class I overload * operator, this will default in
        // c++ to hiding all previous * overloading, so we have to re-expose it
        using base__::operator*;
        using index = unsigned;
        using value_type = number;
        using type_ref = number &;
        using const_type_ref = const number &;
        using matrix_ref = mat4<number> &;
        using const_matrix_ref = const mat4<number> &;
        using vertex3 = nitrogl::vertex3<number>;
        using vertex4 = nitrogl::vertex4<number>;

        static mat4 identity() { return mat4(); }

        static
        mat4 translate(const_type_ref tx, const_type_ref ty, const_type_ref tz) {
            mat4 mat;
            mat(0,3) = tx; mat(1,3) = ty; mat(2,3) = tz;
            return mat;
        }

        static
        mat4 scale(const_type_ref sx, const_type_ref sy, const_type_ref sz) {
            mat4 mat;
            mat[0] = sx; mat[5] = sy; mat[10] = sz;
            return mat;
        }

        static
        mat4 rotation(const_type_ref angle, const vertex3 &axis) {
            mat4 mat{};
            const vertex3 ax = axis.normalize();
            const number s = nitrogl::math::sin(angle);
            const number c = nitrogl::math::cos(angle);
            const number c1 = number(1) - c;
            const number &x = ax.x;
            const number &y = ax.y;
            const number &z = ax.z;

            mat.setRow(0, {x * x * c1 + c, x * y * c1 - z * s, x * z * c1 + y * s });
            mat.setRow(1, {x * y * c1 + z * s, y * y * c1 + c, y * z * c1 - x * s });
            mat.setRow(2, {x * z * c1 - y * s, y * z * c1 + x * s, z * z * c1 + c });

            return mat;
        }

        ///////////////////////////////////////////////////////////////////////////////
        // convert rotation angles (degree) to 4x4 matrix
        // NOnumberE: the angle is for orbit camera, so yaw angle must be reversed before
        // matrix computation.
        //
        // numberhe order of rotation is Roll->Yaw->Pitch (Rx*Ry*Rz)
        // Rx: rotation about X-axis, pitch
        // Ry: rotation about Y-axis, yaw(heading)
        // Rz: rotation about Z-axis, roll
        //    Rx           Ry          Rz
        // |1  0   0| | Cy  0 Sy| |Cz -Sz 0|   | CyCz        -CySz         Sy  |
        // |0 Cx -Sx|*|  0  1  0|*|Sz  Cz 0| = | SxSyCz+CxSz -SxSySz+CxCz -SxCy|
        // |0 Sx  Cx| |-Sy  0 Cy| | 0   0 1|   |-CxSyCz+SxSz  CxSySz+SxCz  CxCy|
        ///////////////////////////////////////////////////////////////////////////////
        static
        mat4 transform(const vertex3 & rotation = {0, 0, 0},
                       const vertex3 & translation = {0, 0, 0},
                       const vertex3 & scale = {1, 1, 1}) {
            mat4 result;
            number sx, sy, sz, cx, cy, cz;
            vertex3 vec;
            // rotation angle about X-axis (pitch)
            sx = nitrogl::math::sin(rotation.x);
            cx = nitrogl::math::cos(rotation.x);
            // rotation angle about Y-axis (yaw)
            sy = nitrogl::math::sin(rotation.y);
            cy = nitrogl::math::cos(rotation.y);
            // rotation angle about Z-axis (roll)
            sz = nitrogl::math::sin(rotation.z);
            cz = nitrogl::math::cos(rotation.z);

            // determine left vector
            vertex3 left     { cy * cz * scale.x,  (sx * sy * cz + cx * sz) * scale.x, (-cx * sy * cz + sx * sz) * scale.x};
            vertex3 up       {-cy * sz * scale.y, (-sx * sy * sz + cx * cz) * scale.y, (cx * sy * sz + sx * cz) * scale.y};
            vertex3 forward  { sy * scale.z,       -sx * cy * scale.z,                 cx * cy * scale.z};

            result.setColumn(0, left);
            result.setColumn(1, up);
            result.setColumn(2, forward);
            result.setColumn(3, translation);

            return result;
        }


         static
         matrix_ref fast_3x3_in_place_transpose(matrix_ref mat) {
            // very fast 3x3 block in-place transpose of a 4x4 matrix
            number tmp;
            tmp = mat[1];  mat[1] = mat[4];  mat[4] = tmp;
            tmp = mat[2];  mat[2] = mat[8];  mat[8] = tmp;
            tmp = mat[6];  mat[6] = mat[9];  mat[9] = tmp;
            return mat;
        }

        mat4() { fill_identity(); };
        template<class Iterable>
        mat4(const Iterable & list) : base__(list) {}
        mat4(const_type_ref fill_value) : base__(fill_value) {}
        mat4(const base__ & mat) : base__(mat) {}
        template<typename number2>
        mat4(const matrix<number2, 4, 4, column_major> & mat) : base__(mat) {}
        template<typename number2>
        mat4(const mat3<number2> & mat) : base__() {
            fill_identity();
            // todo:: optimize this to avoid (r,c) indexing = without multiplication
            for (int ix = 0; ix < 3; ++ix)
                for (int jx = 0; jx < 3; ++jx)
                    this->operator()(ix, jx) = mat(ix, jx);
        }
        virtual ~mat4() = default;

        void fill_diagonal(const_type_ref value) {
            index next = 0;
            for (index row = 0; row < this->rows(); ++row, next+=4)
                this->_data[next++] = value;
        }

        matrix_ref fill_identity() {
            this->fill(0);
            number fill_one{1};
            fill_diagonal(fill_one);
            return *this;
        }

        void setColumn(const index column, const vertex3 & val) {
            auto & me = *this;
            me(0,column)=val.x; me(1,column)=val.y; me(2,column)=val.z;
        }

        void setRow(const index row, const vertex3 & val) {
            auto & me = *this;
            me(row,0)=val.x; me(row,1)=val.y; me(row,2)=val.z;
        }

        vertex4 operator*(const vertex4 & point) const {
            constexpr bool c = column_major; // much faster
            vertex4 res;
            const auto & m = (*this);
            res.x = m[0]*point.x      + m[c?4:1]*point.y  + m[c?8:2]*point.z   + m[c?12:3]*point.w;
            res.y = m[c?1:4]*point.x  + m[5]*point.y      + m[c?9:6]*point.z   + m[c?13:7]*point.w;
            res.z = m[c?2:8]*point.x  + m[c?6:9]*point.y  + m[10]*point.z      + m[c?14:11]*point.w;
            res.w = m[c?3:12]*point.x + m[c?7:13]*point.y + m[c?11:14]*point.z + m[15]*point.w;
            return res;
        }

        bool isIdentity() const {
            number zero=number{0}, one{1};
            return (
                    this->_data[0]==one   && this->_data[1]==zero  && this->_data[2]==zero  && this->_data[3]==zero  &&
                    this->_data[4]==zero  && this->_data[5]==one   && this->_data[6]==zero  && this->_data[7]==zero  &&
                    this->_data[8]==zero  && this->_data[9]==zero  && this->_data[10]==one  && this->_data[11]==zero &&
                    this->_data[12]==zero && this->_data[13]==zero && this->_data[14]==zero && this->_data[15]==one);
        }
    };

    using mat4f = mat4<float>;
}