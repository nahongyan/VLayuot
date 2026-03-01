#ifndef VLAYOUT_FRAMEWORK_H
#define VLAYOUT_FRAMEWORK_H

/**
 * @file framework.h
 * @brief VLayout 框架入口文件
 *
 * 本文件统一包含所有 VLayout 模块，提供一站式导入入口。
 *
 * ## 使用示例
 * \code
 * #include <vlayout/framework.h>
 * using namespace VLayout;
 *
 * class MyDelegate : public DelegateController {
 * public:
 *     MyDelegate() {
 *         addComponent(makeComponent<LabelComponent>("title"));
 *         addComponent(makeComponent<ButtonComponent>("btn"));
 *
 *         setLayout(HBox(16, 8, 16, 8, 12, {
 *             Stretch("title"),
 *             Item("btn", {60, 28}),
 *         }));
 *
 *         bindTo("title").text(Qt::DisplayRole);
 *         onClick("btn", [](auto& idx, auto*) { qDebug() << idx.data(); });
 *
 *         setFixedSizeHint({400, 48});
 *     }
 * };
 * \endcode
 */

#include "vlayout/component.h"
#include "vlayout/binding.h"
#include "vlayout/layoutdescriptor.h"
#include "vlayout/delegatecontroller.h"
#include "vlayout/components.h"

namespace VLayout {

/**
 * @brief 便捷创建组件的工厂函数
 *
 * 创建指定类型的组件并返回共享指针。
 *
 * @tparam T 组件类型
 * @tparam args 构造函数参数类型
 * @param args 传递给组件构造函数的参数
 * @return 组件的共享指针
 *
 * @code
 * auto label = makeComponent<LabelComponent>("title");
 * auto button = makeComponent<ButtonComponent>("btn");
 * \endcode
 */
template<typename T, typename... Args>
std::shared_ptr<T> makeComponent(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

} // namespace VLayout

#endif // VLAYOUT_FRAMEWORK_H
