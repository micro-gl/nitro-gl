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

    template<typename number>
    struct rect_t {
        using cref = const number &;
        number left, top, right, bottom;
        explicit rect_t(cref left=number(0), cref top=number(0),
                        cref right=number(0), cref bottom=number(0)) :
                        left{left}, top{top}, right{right}, bottom{bottom}{}
        rect_t(const rect_t& r) : rect_t{r.left, r.top, r.right, r.bottom} {}
        rect_t & translate(const number l, const number t) {
            left+=l; right+=l; top+=t; bottom+=t;
            return *this;
        }
        rect_t &operator=(const rect_t& r) {
            left=r.left; top=r.top; right=r.right; bottom=r.bottom;
            return *this;
        }
        bool operator==(const rect_t& r) {
            return left==r.left && top=r.top && right=r.right && bottom=r.bottom;
        }
        bool operator!=(const rect_t& r) { return !(*this==r); }
        // test inclusion
        bool operator<(const rect_t& r) {
            return left>r.left && top>r.top && right<r.right && bottom<r.bottom;
        }
        bool operator<=(const rect_t& r) {
            return left>=r.left && top>=r.top && right<=r.right && bottom<=r.bottom;
        }
        bool operator>(const rect_t& r) { return !((*this)<=r); }
        bool operator>=(const rect_t& r) { return !((*this)<r); }
        rect_t intersect(const rect_t & r2) const {
            auto l = left>r2.left ? left : r2.left;
            auto t = top>r2.top ? top : r2.top;
            auto r = right < r2.right ? right : r2.right;
            auto b = bottom < r2.bottom ? bottom : r2.bottom;
            return rect_t{l, t, r, b};
        }
        bool intersects(const rect_t & r2) const { return !intersect(r2).empty(); }
        number width() const { return right-left; }
        number height() const { return bottom-top; }
        bool empty() const { return width()<=number(0) || height()<=number(0); }
    };

    using rectf = rect_t<float>;
    using rect_i = rect_t<int>;
}