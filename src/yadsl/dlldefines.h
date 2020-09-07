#ifndef __YADSL_DLLDEFINES_H__
#define __YADSL_DLLDEFINES_H__

/* DLL definitions for exporting and importing functions */

#if defined(_MSC_VER)
    /* Microsoft */
    #define YADSL_EXPORT __declspec(dllexport)
    #define YADSL_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    /* GCC */
    #define YADSL_EXPORT __attribute__((visibility("default")))
    #define YADSL_IMPORT
#else
    /* do nothing and hope for the best? */
    #define YADSL_EXPORT
    #define YADSL_IMPORT
    #error "Unknown dynamic link import/export semantics."
#endif

#endif
