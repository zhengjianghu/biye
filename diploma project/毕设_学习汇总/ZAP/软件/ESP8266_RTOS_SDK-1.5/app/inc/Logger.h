/**
 * \brief 适用于不同模块的共通的日志输出模块对外接口头文件
 * \copyright inSona Co.,Ltd.
 * \author  
 * \file  Logger.h
 *
 * Dependcy:
 *       Platform.stdint
 *       Platform.macrodefine
 *       Platform.string
 *       Platform.stdio
 * 
 * \date 2013-10-17   zhouyong 新建文件
**/

#ifndef LOGGER_H
#define LOGGER_H


/*standard type define*/
#include "Platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 *   \brief    日志工厂属性结构体
**/
typedef struct {
    char_t * buf;       /**< 日志字符串指针 */
    char_t * name;      /**< 模块名 */
    uint16_t bufSize;   /**< 存储日志字符串长度 */
    uint8_t  level;     /**< 模块日志等级 */
} Logger_ST;

/**
 *   \brief   : 日志工厂配置结构体
**/
typedef struct {
    void (*putsAppender)   (char_t * s);  /**< 以puts(char*)方式输出 */
    void (*putcharAppender)(char_t c);          /**< 以putchar(char)方式输出 */
    void (*getTimeString)(char_t * s, int32_t maxlen);            /**< 获取当前时间结构体函数 */
    void (*getThreadName)(char_t * s, int32_t maxlen);            /**< 获取当前进程名结构体函数 */
    uint8_t maxTimeStringLength;                /**< 时间字符串长度 */
    uint8_t maxThreadNameLength;                /**< 进程名字符串长度 */
    uint8_t maxModuleNameLength;                /**< 模块名字符串长度 */
    bool_t  levelStringEnable;                  /**< 输出等级字符串 */
} LoggerConfig_ST;


/**
 *   \brief    日志初始配置导入
**/
void Logger_Init(const LoggerConfig_ST * const config);

/**
 *   \brief   组日志等级设置
 *   param [in]                   level   组日志等级
**/
void Logger_SetGlobalLevel(uint8_t level);

/**
 *   \brief   模块日志等级修改
 *   param [in]                  *logger  目标模块名和原日志等级
 *   param [in]                   level   修改日志等级为 level
**/
void Logger_SetLevel(Logger_ST * logger, uint8_t level);

/**
 *   \brief    标准日志输出的对外接口
 *   param [in]                   len    buf长度
**/
void Logger_Print(Logger_ST * logger, uint8_t level, bool_t skipHead, char_t * fmt, ...);

/**
 *   \brief                       将指定地址的数据16进制的格式输出
 *   param [in]                   level  模块等级
 *   param [in]                   name   模块名
 *   param [in]                   buf    数据地址
 *   param [in]                   len    输出16进制字符的长度
**/
void Logger_HexDump(Logger_ST * const logger, const uint8_t level, char_t * const name, const uint8_t * buf, uint32_t len);

#define LOG_OFF     0U      /**<  模块日志等级 off */
#define LOG_ERROR   1U      /**<  模块日志等级 error */
#define LOG_WARN    2U      /**<  模块日志等级 waring */
#define LOG_INFO    3U      /**<  模块日志等级 info */
#define LOG_DEBUG   4U      /**<  模块日志等级 debug */
#define LOG_TRACE   5U      /**<  模块日志等级 trace */

/**  模块日志属性设置宏 */
#define DEFINE_LOGGER(modename,bufsize,level)   \
        static char_t __thisLoggerBuf[(bufsize)]; \
        Logger_ST __##modename##Logger = {__thisLoggerBuf, #modename, bufsize, level}; \
        static Logger_ST *__thisLogger = &__##modename##Logger;
/**  引用已有模块日志属性宏 */
#define USE_LOGGER(modename) \
		extern Logger_ST __##modename##Logger; \
		static Logger_ST *__thisLogger = &__##modename##Logger;
/**  修改已有模块日志属性宏 */
#define LOGGER_SETLEVEL(modename, level) \
		extern Logger_ST __##modename##Logger; \
		Logger_SetLevel(&__##modename##Logger, level);



/** Error 等级对外日志输出接口 */
#define LogError(fmt, ...)  Logger_Print(__thisLogger, LOG_ERROR, FALSE, (fmt), ##__VA_ARGS__)
/**  Warn 等级对外日志输出接口 */
#define LogWarn(fmt, ...)   Logger_Print(__thisLogger, LOG_WARN , FALSE, fmt, ##__VA_ARGS__)
/** Info 等级对外日志输出接口 */
#define LogInfo(fmt, ...)   Logger_Print(__thisLogger, LOG_INFO , FALSE, fmt, ##__VA_ARGS__)
/** Debug 等级对外日志输出接口 */
#define LogDebug(fmt, ...)  Logger_Print(__thisLogger, LOG_DEBUG, FALSE, fmt, ##__VA_ARGS__)
/** Trace 等级对外日志输出接口 */
#define LogTrace(fmt, ...)  Logger_Print(__thisLogger, LOG_TRACE, FALSE, fmt, ##__VA_ARGS__)
/** 不带头信息的日志输出接口 */
#define LogAppend(level, fmt, ...)  Logger_Print(__thisLogger, level, TRUE, fmt, ##__VA_ARGS__)
/**  指定数据地址内容转换成16进制格式输出接口 */
#define LogHexDump(level, name, buf, len)  Logger_HexDump(__thisLogger, level, name, buf, len)
/**   模块日志等级设置对外接口 */
#define LogSetLevel(level)  Logger_SetLevel(__thisLogger, level);



#ifdef __cplusplus
}

#endif

#endif
