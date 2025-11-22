#pragma once

/*
* 
*                __     __   ________   _______     ______
*               /  |   /  | /        | /       \   /      \
*               ## |   ## | ########/  #######  | /######  |
*               ## |   ## | ## |__     ## |__## | ## |__## |
*               ##  \ /##/  ##    |    ##    ##<  ##    ## |
*                ##  /##/   #####/     #######  | ######## |
*                 ## ##/    ## |_____  ## |  ## | ## |  ## |
*                  ###/     ##       | ## |  ## | ## |  ## |
*                   #/      ########/  ##/   ##/  ##/   ##/  VER 1.0.0
*            
*                     C++20 Modern Vulkan Graphics Engine
* 
* The MIT License (MIT)
* 
* Copyright (c) 2025 dandevlog0206
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
* 
*/

#define VERA_VERSION_MAJOR 1
#define VERA_VERSION_MINOR 0
#define VERA_VERSION_PATCH 0

// namespace
#define VERA_NAMESPACE_BEGIN namespace vr {
#define VERA_NAMESPACE_END }
#define VERA_OS_NAMESPACE_BEGIN namespace os {
#define VERA_OS_NAMESPACE_END }
#define VERA_SCENE_NAMESPACE_BEGIN namespace scene {
#define VERA_SCENE_NAMESPACE_END }
#define VERA_SDF_NAMESPACE_BEGIN namespace sdf {
#define VERA_SDF_NAMESPACE_END }
#define VERA_PRIV_NAMESPACE_BEGIN namespace priv {
#define VERA_PRIV_NAMESPACE_END }

// utilities
#define VERA_LENGTHOF(arr) (sizeof(arr) / sizeof(arr[0]))

#define VERA_NOCOPY(cls) cls(const cls&) = delete;
#define VERA_NOMOVE(cls) cls(cls&&) = delete;

#define VERA_SPAN(cont) cont.begin(), cont.end()
#define VERA_SPAN_ARRAY(ptr, size) (ptr), (ptr + size)

#define VERA_MAKE_VERSION(major, minor, patch) \
    ((((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch)))

#define VERA_VERSION VERA_MAKE_VERSION(VERA_VERSION_MAJOR, VERA_VERSION_MINOR, VERA_VERSION_PATCH)

#define VERA_CONCAT_TOKEN_IMPL(a, b) a ## b
#define VERA_CONCAT_TOKEN(a, b) VERA_CONCAT_TOKEN_IMPL(a, b)

#define VERA_MEMBER_PADDING(type)                     \
private:                                              \
    type VERA_CONCAT_TOKEN(_padding_, __LINE__) = {}; \
public:

#define VERA_ENUM_COUNT(enum_type) \
    static_cast<uint32_t>(enum_type::__COUNT__)

#define VERA_KIB(x) (1024 * x)
#define VERA_MIB(x) (1024 * 1024 * x)
#define VERA_GIB(x) (1024 * 1024 * 1024 * x)
#define VERA_KB(x)  (1000 * x)
#define VERA_MB(x)  (1000 * 1000 * x)
#define VERA_GB(x)  (1000 * 1000 * 1000 * x)

#ifdef _DEBUG
#define VERA_IS_DEBUG true
#else
#define VERA_IS_DEBUG false
#endif

// keywords
#define VERA_INLINE inline
#define VERA_FORCEINLINE __forceinline
#define VERA_NOEXCEPT noexcept
#define VERA_CONSTEXPR constexpr

#ifdef _MSC_VER
#define VERA_NO_UNIQUE_ADRESS [[msvc::no_unique_address]]
#else
#define VERA_NO_UNIQUE_ADRESS [[no_unique_address]]
#endif
#define VERA_DEPRECATED [[deprecated]]
#define VERA_DEPRECATED_MSG(msg) [[deprecated(msg)]]
#define VERA_NODISCARD [[nodiscard]]
#define VERA_FALLTHROUGH [[fallthrough]]
#define VERA_NORETURN [[noreturn]]

// defines
#define VERA_ENUM : uint32_t
#define VERA_ENUM64 : uint64_t
#define VERA_FLAG_BITS : uint32_t
#define VERA_FLAG_BITS64 : uint64_t
#define VERA_VK_ABI_COMPATIBLE
