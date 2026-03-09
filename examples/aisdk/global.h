#ifndef AISDK_GLOBAL_H
#define AISDK_GLOBAL_H

/**
 * @file global.h
 * @brief AI SDK 全局定义
 */

#include <QtGlobal>

// 导出宏定义
#if defined(AISDK_SHARED)
#  if defined(AISDK_BUILD)
#    define AISDK_EXPORT Q_DECL_EXPORT
#  else
#    define AISDK_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define AISDK_EXPORT
#endif

#endif // AISDK_GLOBAL_H
