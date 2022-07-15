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
using trivial = safe_type<T, Tag, impl::Category::Trivial>;
template <typename T, typename Tag>
using ordered = safe_type<T, Tag, impl::Category::Ordered>;
template <typename T, typename Tag>
using scalar = safe_type<T, Tag, impl::Category::Scalar>;
}
}
