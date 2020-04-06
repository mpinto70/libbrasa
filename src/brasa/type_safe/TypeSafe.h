// inspired by https://github.com/rollbear/strong_type
#pragma once

#include <brasa/type_safe/TypeSafeImpl.h>

#include <array>
#include <cstring>
#include <type_traits>

namespace brasa {
namespace type_safe {
// Important definitions. You should use one of the below types
// trivial: [ == and != ]
// ordered: trivial + [ < > <= >= ]
// scalar: ordered + [ + += - -= * (value) / (value) ]
template <typename T, typename Tag>
using trivial = base_type<T, Tag, impl::Category::Trivial>;
template <typename T, typename Tag>
using ordered = base_type<T, Tag, impl::Category::Ordered>;
template <typename T, typename Tag>
using scalar = base_type<T, Tag, impl::Category::Scalar>;

/// cast from underlying to wrapped type
template <typename TYPE>
constexpr TYPE type_cast(typename TYPE::underlying_type value) noexcept {
    return TYPE{ value };
}

template <typename TYPE>
constexpr const typename TYPE::underlying_type& value_cast(const TYPE& t) noexcept {
    return t.value;
}
}
}
