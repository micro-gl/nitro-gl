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
#ifndef NITROGL_USE_EXTERNAL_MICRO_TESS
#include "micro-tess/include/micro-tess/path.h"
#include "micro-tess/include/micro-tess/dynamic_array.h"
#else
#include <micro-tess/path.h>
#include <micro-tess/dynamic_array.h>
#endif
#include "traits.h"

namespace nitrogl {

    /**
     * Path alias (float point version)
     * @tparam container_template_type the container template type
     * @tparam Allocator the memory allocator for the container and tessellation
     */
    template <template<typename...> class container_template_type=dynamic_array,
              class Allocator=nitrogl::std_rebind_allocator<>>
    using path = microtess::path<float, container_template_type, Allocator>;
}