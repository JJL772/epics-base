/*************************************************************************\
* Copyright (c) 2008 UChicago Argonne LLC, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* SPDX-License-Identifier: EPICS
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/

/**
 * \file   compilerDependencies.h
 * \author Jeffrey O. Hill johill@lanl.gov
 * \brief  Compiler specific declarations
 *
 */

#ifndef compilerDependencies_h
#define compilerDependencies_h

#include "compilerSpecific.h"

#ifdef __cplusplus

/*
 * usage: epicsPlacementDeleteOperator (( void *, myMemoryManager & ))
 */
#if defined ( CXX_PLACEMENT_DELETE )
#   define epicsPlacementDeleteOperator(X) void operator delete X;
#else
#   define epicsPlacementDeleteOperator(X)
#endif

#endif /* __cplusplus */


#ifndef EPICS_PRINTF_STYLE
/*
 * No format-string checking
 */
#   define EPICS_PRINTF_STYLE(f,a)
#endif

#ifndef EPICS_DEPRECATED
/*
 * No deprecation markers
 */
#define EPICS_DEPRECATED
#endif

#ifndef EPICS_UNUSED
#   define EPICS_UNUSED
#endif

/*
 * Clang fortify diagnostics
 */
#ifndef EPICS_DIAGNOSE_AS
#   define EPICS_DIAGNOSE_AS(...)
#endif

#ifndef EPICS_DIAGNOSE_AS_STRNCASECMP
#   define EPICS_DIAGNOSE_AS_STRNCASECMP(_1, _2, _3)
#endif

#ifndef EPICS_DIAGNOSE_AS_STRDUP
#   define EPICS_DIAGNOSE_AS_STRDUP(_1)
#endif

#ifndef EPICS_DIAGNOSE_AS_STRNDUP
#   define EPICS_DIAGNOSE_AS_STRNDUP(_1, _2)
#endif

#ifndef EPICS_DIAGNOSE_AS_SNPRINTF
#   define EPICS_DIAGNOSE_AS_SNPRINTF(_1, _2_, _3)
#endif

#ifndef EPICS_DIAGNOSE_AS_VSNPRINTF
#   define EPICS_DIAGNOSE_AS_VSNPRINTF(_1, _2, _3, _4)
#endif

#ifndef EPICS_DIAGNOSE_AS_STRCASECMP
#   define EPICS_DIAGNOSE_AS_STRCASECMP(_1, _2)
#endif

#ifndef EPICS_DIAGNOSE_AS_PRINTF
#   define EPICS_DIAGNOSE_AS_PRINTF(_1)
#endif

#ifndef EPICS_DIAGNOSE_AS_VPRINTF
#   define EPICS_DIAGNOSE_AS_VPRINTF(_1, _2)
#endif

#ifndef EPICS_FUNCTION
#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)) || (defined(__cplusplus) && __cplusplus>=201103L)
#  define EPICS_FUNCTION __func__
#else
/* Expands to a 'const char*' which describes the name of the current function scope */
#  define EPICS_FUNCTION ("<unknown function>")
#endif
#endif

#ifndef EPICS_PRINTF_FMT
/*
 * No format-string checking annotation
 */
#   define EPICS_PRINTF_FMT(a) a
#endif

#endif  /* ifndef compilerDependencies_h */
