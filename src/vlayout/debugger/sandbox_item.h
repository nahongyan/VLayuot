#ifndef VLAYOUT_SANDBOX_ITEM_H
#define VLAYOUT_SANDBOX_ITEM_H

/**
 * @file sandbox_item.h
 * @brief 沙盒布局项数据结构
 */

#include <QString>

namespace VLayout {

/**
 * @struct SandboxItem
 * @brief 沙盒中单个布局项的数据
 */
struct SandboxItem
{
    QString id;             ///< 项标识

    // 主方向参数（由布局方向决定使用哪个）
    int sizeHint = 0;       ///< 主方向首选尺寸
    int minSize = 0;        ///< 主方向最小尺寸
    int maxSize = 1000000;  ///< 主方向最大尺寸
    int stretch = 0;        ///< 主方向拉伸因子

    // 交叉方向参数（垂直布局用 width，水平布局用 height）
    int crossSizeHint = 0;       ///< 交叉方向首选尺寸 (0 表示使用容器尺寸)
    int crossMinSize = 0;        ///< 交叉方向最小尺寸
    int crossMaxSize = 1000000;  ///< 交叉方向最大尺寸

    bool isSpacing = false; ///< 是否为间隔项

    // 计算结果（由布局算法填充）
    int pos = 0;            ///< 主方向位置
    int size = 0;           ///< 主方向计算后尺寸
    int crossSize = 0;      ///< 交叉方向计算后尺寸
    bool isCompressed = false;  ///< 是否被压缩
    bool isOverflow = false;    ///< 是否溢出（小于最小尺寸）
};

} // namespace VLayout

#endif // VLAYOUT_SANDBOX_ITEM_H
