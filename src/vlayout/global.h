#ifndef VLAYOUT_GLOBAL_H
#define VLAYOUT_GLOBAL_H

/**
 * @file global.h
 * @brief VLayout 框架全局定义和版本信息
 *
 * 本文件定义了 VLayout 框架的全局宏、版本信息和导出声明。
 */

#include <QtGlobal>

/*!
 * \namespace VLayout
 * \brief 轻量级虚拟布局框架，提供与 Qt 兼容的布局算法
 *
 * VLayout 提供了一套完整的布局管理系统，使用与 Qt QBoxLayout 相同的算法，
 * 但不创建实际的 QWidget 实例。这使得它非常适合自定义绘制场景、
 * 列表视图委托以及其他需要布局计算但不需要实际控件开销的情况。
 *
 * \section features 主要特性
 * - 与 Qt QBoxLayout 算法完全一致
 * - 支持水平 (HBox) 和垂直 (VBox) 布局
 * - 支持 minimum/maximum 尺寸约束
 * - 支持 stretch 因子和对齐方式
 * - 支持边距和间距
 * - 支持 Spacer 项实现弹性布局
 * - 声明式数据绑定系统
 * - 声明式布局描述系统
 *
 * \section usage 基本用法
 * \code
 * #include <vlayout/framework.h>
 *
 * // 创建水平布局
 * auto layout = std::make_shared<VLayout::HBoxLayout>();
 * layout->setSpacing(10);
 * layout->setContentsMargins(10, 10, 10, 10);
 *
 * // 添加组件
 * auto label = VLayout::createLabel("title", "Title");
 * label->setMinimumWidth(50);
 * layout->addItem(label);
 *
 * layout->addItem(VLayout::createStretch());
 *
 * auto button = VLayout::createButton("btn", "Click");
 * layout->addItem(button);
 *
 * // 激活布局
 * layout->setGeometry(QRect(0, 0, 400, 50));
 * layout->activate();
 *
 * // 获取最终位置用于绘制
 * QRect labelRect = label->finalRect();
 * \endcode
 */

// ============================================================================
// 版本信息
// ============================================================================

namespace VLayout {

/*!
 * \brief 库版本信息
 *
 * 提供编译期和运行时的版本查询支持。
 */
namespace Version {
    /// 主版本号（不兼容的 API 变更）
    constexpr int Major = 1;
    /// 次版本号（向后兼容的功能新增）
    constexpr int Minor = 0;
    /// 补丁版本号（向后兼容的问题修复）
    constexpr int Patch = 0;
    /// 完整版本字符串
    constexpr const char* String = "1.0.0";
}

} // namespace VLayout

// ============================================================================
// 导出宏定义
// ============================================================================

#if defined(VLAYOUT_SHARED)
#  if defined(VLAYOUT_BUILD)
     // 构建库时导出符号
#    define VLAYOUT_EXPORT Q_DECL_EXPORT
#  else
     // 使用库时导入符号
#    define VLAYOUT_EXPORT Q_DECL_IMPORT
#  endif
#else
   // 静态链接时不使用导出宏
#  define VLAYOUT_EXPORT
#endif

#endif // VLAYOUT_GLOBAL_H
