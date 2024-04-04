// inspired by https://github.com/rollbear/strong_type
#pragma once

#include <brasa/safe_type/SafeTypeImpl.h>

#include <array>
#include <cstring>
#include <type_traits>

namespace brasa::safe_type {
// Important definitions. You should use one of the below types
// Trivial: [ == and != ]
// Ordered: trivial + [ < > <= >= ]
// Scalar: ordered + [ + += - -= * (value) / (value) ]
template <typename T, typename Tag>
using Trivial = SafeType<T, Tag, impl::Category::Trivial>;
template <typename T, typename Tag>
using Ordered = SafeType<T, Tag, impl::Category::Ordered>;
template <typename T, typename Tag>
using Scalar = SafeType<T, Tag, impl::Category::Scalar>;
} // namespace brasa::safe_type
