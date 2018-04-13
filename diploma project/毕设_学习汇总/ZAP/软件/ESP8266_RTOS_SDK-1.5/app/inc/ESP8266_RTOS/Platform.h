/**    
 *

 */

#ifndef PLATFORM_H
#define PLATFORM_H


/*
 * Platform.h function
 * 1. stdint : char_t intx_t uintx_t bool_t (不使用size_t)
 * 2. macrodefine :   TRUE  FALSE NULL assert
 * 3. string : memcpy memset memcmp strncpy strnlen strncmp (不使用strlen这套不安全的api)
 * 4. stdarg : va_list va_start va_end
 * 5. stdio : snprintf vsnprintf (不使用strlen这套不安全的api)
 */
#include "esp_common.h" /*size_t */
#include <string.h>     /* memcpy memset memcmp strncpy strncmp strnlen */
#include <stdarg.h>     /* va_list va_start */
#include <stdio.h>      /* snprintf vsnprintf */
#include <ctype.h>

#ifdef __cplusplus
extern "C" 
{
#endif


typedef char                char_t;     /**< 数据类型定义：字符类型*/

typedef signed char         int8_t;     /**< 数据类型定义：8位有符号整型*/

typedef unsigned char       uint8_t;    /**< 数据类型定义：8位无符号整型*/

typedef signed short        int16_t;    /**< 数据类型定义：16位有符号整型*/

typedef unsigned short      uint16_t;  /**< 数据类型定义：16位无符号整型*/

typedef signed int         int32_t;    /**< 数据类型定义：32位有符号整型*/

typedef unsigned int       uint32_t;   /**< 数据类型定义：32位无符号整型*/

typedef signed long long    int64_t;     /**< 数据类型定义：64位有符号整型*/

typedef unsigned long long int  uint64_t;   /**< 数据类型定义：64位无符号整型*/

typedef unsigned char           bool_t;   /**< 数据类型定义：布尔类型*/

#undef  NULL
#define NULL    ((void *)0)     /**< 空指针*/

/**< 断言配置 宏开关，未定义_DEBUG_宏 assert输出为空，定义_DEBUG_宏 带参宏输入FALSE，输出行号和文件名*/
#undef assert
#ifndef _DEBUG_
#define assert(e) ((void)0)
#else
#define assert(e) ((e) ? (void)0 : _xassert(__FILE__, __LINE__))
void _xassert(const char *, int);
#endif

/**< static 宏开关，定义_TEST_宏后 STATIC为空*/
#ifdef _TEST_
#define STATIC
#else
#define STATIC 	static
#endif

#ifdef PARASOFT_STATIC_CHECK
#define DOWHILE0(exper)            exper
#else
#define DOWHILE0(exper)            do{exper} while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_H */
