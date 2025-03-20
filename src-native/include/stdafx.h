#ifndef GAME_MAPS_CORE_STDAFX_H
#define GAME_MAPS_CORE_STDAFX_H

#ifdef __cplusplus
  #define CORE_EXTERN_C extern "C"
  #define CORE_EXTERN_C_BEGIN extern "C" {
  #define CORE_EXTERN_C_END   }
  #define REMOVE_IN_CPP(x)
#else
  #define CORE_EXTERN_C
  #define CORE_EXTERN_C_BEGIN
  #define CORE_EXTERN_C_END
  #define REMOVE_IN_CPP(x) x
#endif

#if WIN32
  #ifdef GAME_MAPS_CORE_RELEASE
    #define CORE_PUBLIC __declspec(dllexport)
  #else
    #define CORE_PUBLIC
  #endif

  #define CORE_PRIVATE
#else
  #define CORE_PUBLIC __attribute__((visibility("default")))
  #define CORE_PRIVATE __attribute__((visibility("hidden")))
#endif

#endif // GAME_MAPS_CORE_STDAFX_H