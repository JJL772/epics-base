/*************************************************************************\
* Copyright (c) 2008 UChicago Argonne LLC, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* SPDX-License-Identifier: EPICS
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/

/*
 * Author:
 * Jeffrey O. Hill
 * johill@lanl.gov
 */

#ifndef compilerSpecific_h
#define compilerSpecific_h

#ifndef __clang__
#   error compiler/clang/compilerSpecific.h is only for use with the clang compiler
#endif

#if __has_attribute(always_inline)
#define EPICS_ALWAYS_INLINE __inline__ __attribute__((always_inline))
#else
#define EPICS_ALWAYS_INLINE __inline__
#endif

/* Expands to a 'const char*' which describes the name of the current function scope */
#define EPICS_FUNCTION __PRETTY_FUNCTION__

/*
 * Allows the compiler to apply fortify diagnostics to marked functions.
 * Introduced in clang 14.
 * Ref: https://clang.llvm.org/docs/AttributeReference.html#diagnose-as-builtin
 */
#if __has_attribute(diagnose_as_builtin)
#define EPICS_DIAGNOSE_AS(...) __attribute__((diagnose_as_builtin(__VA_ARGS__)))

#if __has_builtin(__builtin_strncasecmp)
#define EPICS_DIAGNOSE_AS_STRNCASECMP(_1, _2, _3) EPICS_DIAGNOSE_AS(__builtin_strncasecmp, _1, _2, _3)
#endif

#if __has_builtin(__builtin_strdup)
#define EPICS_DIAGNOSE_AS_STRDUP(_1) EPICS_DIAGNOSE_AS(__builtin_strdup, _1)
#endif

#if __has_builtin(__builtin_strndup)
#define EPICS_DIAGNOSE_AS_STRNDUP(_1, _2) EPICS_DIAGNOSE_AS(__builtin_strndup, _1, _2)
#endif

#if __has_builtin(__builtin_snprintf)
#define EPICS_DIAGNOSE_AS_SNPRINTF(_1, _2, _3) EPICS_DIAGNOSE_AS(__builtin_snprintf, _1, _2, _3)
#endif

#if __has_builtin(__builtin_vsnprintf)
#define EPICS_DIAGNOSE_AS_VSNPRINTF(_1, _2, _3, _4) EPICS_DIAGNOSE_AS(__builtin_vsnprintf, _1, _2, _3, _4)
#endif

#if __has_builtin(__builtin_strcasecmp)
#define EPICS_DIAGNOSE_AS_STRCASECMP(_1, _2) EPICS_DIAGNOSE_AS(__builtin_strcasecmp, _1, _2)
#endif

#if __has_builtin(__builtin_printf)
#define EPICS_DIAGNOSE_AS_PRINTF(_1) EPICS_DIAGNOSE_AS(__builtin_printf, _1)
#endif

#if __has_builtin(__builtin_vprintf)
#define EPICS_DIAGNOSE_AS_VPRINTF(_1, _2) EPICS_DIAGNOSE_AS(__builtin_vprintf, _1, _2)
#endif

#endif /* __has_attribute(diagnose_as_builtin) */

#ifdef __cplusplus

/*
 * CXX_PLACEMENT_DELETE - defined if compiler supports placement delete
 */
#define CXX_PLACEMENT_DELETE

#endif /* __cplusplus */

/*
 * __has_attribute() is not supported on all versions of clang yet
 */

/*
 * Enable format-string checking
 */
#define EPICS_PRINTF_STYLE(f,a) __attribute__((format(__printf__,f,a)))

/*
 * Deprecation marker
 */
#define EPICS_DEPRECATED __attribute__((deprecated))

/*
 * Unused marker
 */
#define EPICS_UNUSED __attribute__((unused))

#endif  /* ifndef compilerSpecific_h */
