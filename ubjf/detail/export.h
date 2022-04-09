
#ifndef UBJF_API_H
#define UBJF_API_H

#ifdef UBJF_STATIC_DEFINE
#  define UBJF_API
#  define UBJF_NO_EXPORT
#else
#  ifndef UBJF_API
#    ifdef ubjf_EXPORTS
        /* We are building this library */
#      define UBJF_API __declspec(dllexport)
#    else
        /* We are using this library */
#      define UBJF_API __declspec(dllimport)
#    endif
#  endif

#  ifndef UBJF_NO_EXPORT
#    define UBJF_NO_EXPORT 
#  endif
#endif

#ifndef UBJF_DEPRECATED
#  define UBJF_DEPRECATED __declspec(deprecated)
#endif

#ifndef UBJF_DEPRECATED_EXPORT
#  define UBJF_DEPRECATED_EXPORT UBJF_API UBJF_DEPRECATED
#endif

#ifndef UBJF_DEPRECATED_NO_EXPORT
#  define UBJF_DEPRECATED_NO_EXPORT UBJF_NO_EXPORT UBJF_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef UBJF_NO_DEPRECATED
#    define UBJF_NO_DEPRECATED
#  endif
#endif

#endif /* UBJF_API_H */
