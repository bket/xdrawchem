#pragma once
// Compatibility shim: std::binary_function was fully removed from Apple libc++
// in Xcode 16 / libc++ 19. _LIBCPP_ENABLE_CXX17_REMOVED_FEATURES no longer
// restores it. OpenBabel 3.1.x headers (plugin.h) still require it.
// This header is force-included on Apple builds via CMakeLists.txt.
// On Apple (Xcode 16+/libc++19) binary_function was fully removed.
// On Linux (GCC 13+) it still exists but is deprecated, producing warnings
// from OpenBabel headers we cannot modify.  In both cases we either define
// it (Apple) or suppress the deprecation warning around the OB includes.
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
#elif defined(__GNUC__)
// GCC: push/pop deprecation warnings so OB headers compile cleanly.
// Usage: wrap OB #includes with OB_COMPAT_BEGIN / OB_COMPAT_END.
#  define OB_COMPAT_BEGIN \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#  define OB_COMPAT_END \
    _Pragma("GCC diagnostic pop")
#endif

#ifndef OB_COMPAT_BEGIN
#  define OB_COMPAT_BEGIN
#  define OB_COMPAT_END
#endif
