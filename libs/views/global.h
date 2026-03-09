#ifndef VIEWS_GLOBAL_H
#define VIEWS_GLOBAL_H

/**
 * @file global.h
 * @brief Views 库全局定义
 */

#include <QtGlobal>

// 导出宏定义
#if defined(VIEWS_SHARED)
#  if defined(VIEWS_BUILD)
#    define VIEWS_EXPORT Q_DECL_EXPORT
#  else
#    define VIEWS_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define VIEWS_EXPORT
#endif

#endif // VIEWS_GLOBAL_H
