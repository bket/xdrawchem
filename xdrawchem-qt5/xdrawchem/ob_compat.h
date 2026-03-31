#pragma once
// Compatibility shim: std::binary_function was fully removed from Apple libc++
// in Xcode 16 / libc++ 19. _LIBCPP_ENABLE_CXX17_REMOVED_FEATURES no longer
// restores it. OpenBabel 3.1.x headers (plugin.h) still require it.
// This header is force-included on Apple builds via CMakeLists.txt.
#if defined(__APPLE__)
#  include <functional>
namespace std {
  template<class _Arg1, class _Arg2, class _Result>
  struct binary_function {
    using first_argument_type  = _Arg1;
    using second_argument_type = _Arg2;
    using result_type          = _Result;
  };
}
#endif
