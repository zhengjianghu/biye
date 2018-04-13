/*
 *
 * \brief       common library -- Ring buffer
 * \Copyright (c) 2017 by inSona Corporation. All rights reserved.
 * \author      zhouyong
 * \file        ringbuf16.h
 *
 * General crc16-ccitt function.
 *
 */

/** \addtogroup lib
 * @{ */

/**
 * \defgroup ringbuf Ring buffer library
 * @{
 *
 * The ring buffer library implements ring (circular) buffer where
 * bytes can be read and written independently. A ring buffer is
 * particularly useful in device drivers where data can come in
 * through interrupts.
 *
 */

#ifndef ESP8266_PLATFORM_H
#define ESP8266_PLATFORM_H

#include "Logger.h"

/** \name Master Variable Types
 * These are a set of typedefs to make the size of all variable declarations
 * explicitly known.
 */
//@{
/**
 * @brief A typedef to make the size of the variable explicitly known.
 */
typedef unsigned char  boolean;
typedef unsigned char  int8u;
typedef signed   char  int8s;
typedef unsigned short int16u;
typedef signed   short int16s;
typedef unsigned int   int32u;
typedef signed   int   int32s;
typedef unsigned long long int64u;
typedef signed   long long int64s;
typedef unsigned long  PointerType;
//@} \\END MASTER VARIABLE TYPES

//#include "assert.h"

int32u halCommonGetInt32uMillisecondTick(void);
void simulatedTimePasses(void);
int16u halCommonGetRandom(void);


#define ashTraceEzspVerbose(fmt, ...) do {	\
    if (ashReadConfig(traceFlags) & TRACE_EZSP_VERBOSE) { \
    LogDebug(fmt, ##__VA_ARGS__);	\
    } } while(0)

/**
 * Include string.h for the C Standard Library memory routines used in
 * platform-common.
 */
#include <string.h>

#define SIGNED_ENUM

/**
 * @brief Use the Master Program Memory Declarations from platform-common.h
 */
#define _HAL_USE_COMMON_PGM_

/**
 * @brief Use the Divide and Modulus Operations from platform-common.h
 */
#define _HAL_USE_COMMON_DIVMOD_

/**
 * @brief A definition stating what the endianess of the platform is.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
#define BIGENDIAN_CPU FALSE
#else
  #if defined(__i386__)
    #define BIGENDIAN_CPU  FALSE
  #elif defined(__ARM7__)
    #define BIGENDIAN_CPU  FALSE
  #elif defined(__x86_64__)
    #define BIGENDIAN_CPU  FALSE
  #elif defined(__arm__)
    #if defined(__BIG_ENDIAN)
      #define BIGENDIAN_CPU  TRUE
    #else
      #define BIGENDIAN_CPU  FALSE
    #endif
  #elif defined(__LITTLE_ENDIAN__)
    #define BIGENDIAN_CPU  FALSE
  #elif defined(__BIG_ENDIAN__)
    #define BIGENDIAN_CPU  TRUE
  #elif defined(__APPLE__)
    #define BIGENDIAN_CPU  TRUE
  #else
    #error endianess not defined
  #endif
#endif

#define EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE 3

#define MAIN_FUNCTION_PARAMETERS int argc, char* argv[]

/**
 * @brief Include platform-common.h last to pick up defaults and common definitions.
 */
#define PLATCOMMONOKTOINCLUDE
  #include "hal/micro/generic/compiler/platform-common.h"
#undef PLATCOMMONOKTOINCLUDE


#include <stdarg.h>

#endif /* ESP8266_PLATFORM_H */

/** @}*/
/** @}*/
