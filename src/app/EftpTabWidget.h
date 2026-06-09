#pragma once

#include <QTabWidget>
#include <QTabBar>
#include <QPainter>
#include <QPixmap>
#include "EftpTypes.h"

/**
 * @brief 测试阶段 TabWidget — 状态图标 + 文字颜色
 *
 * 状态显示：
 *   - 未开始：灰色圆点 + 灰色文字
 *   - 运行中：黄色圆点 + 橙色文字
 *   - 通过：  绿色圆点 + 绿色文字
 *   - 失败：  红色圆点 + 红色文字
 */
class EftpTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit EftpTabWidget(QWidget* parent = nullptr)
        : QTabWidget(parent)
    {
        tabBar()->setExpanding(true);
        tabBar()->setDocumentMode(true);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // 设置图标大小（32x32 状态图标）
        tabBar()->setIconSize(QSize(32, 32));
    }

    /**
     * @brief 设置 Tab 的测试状态（图标 + 文字颜色）
     */
    void setTabTestState(int index, TestState state)
    {
        // 生成状态图标（小圆点）
        QIcon icon;
        QColor textColor;
        QColor dotColor;

        switch (state) {
            case TestState::Running:
                dotColor = QColor("#F37E00");
                textColor = QColor("#F37E00");
                break;
            case TestState::Success:
                dotColor = QColor("#01B659");
                textColor = QColor("#01B659");
                break;
            case TestState::Fail:
                dotColor = QColor("#E73C31");
                textColor = QColor("#E73C31");
                break;
            default: // UnStart
                dotColor = QColor("#CCCCCC");
                textColor = QColor("#8D8D8D");
                break;
        }

        // 生成 32x32 状态图标
        QPixmap pixmap(32, 32);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        switch (state) {
            case TestState::Running:
                // 旋转箭头（运行中）
                painter.setPen(QPen(QColor("#F37E00"), 3));
                painter.setBrush(Qt::NoBrush);
                painter.drawArc(4, 4, 24, 24, 0, 270 * 16);
                painter.drawLine(22, 4, 28, 10);
                painter.drawLine(22, 4, 28, 0);
                break;
            case TestState::Success:
                // 绿色对勾（粗线）
                painter.setPen(QPen(QColor("#01B659"), 4));
                painter.drawLine(4, 16, 12, 26);
                painter.drawLine(12, 26, 28, 6);
                break;
            case TestState::Fail:
                // 红色叉号（粗线）
                painter.setPen(QPen(QColor("#E73C31"), 4));
                painter.drawLine(6, 6, 26, 26);
                painter.drawLine(26, 6, 6, 26);
                break;
            default: // UnStart
                // 灰色横线
                painter.setPen(QPen(QColor("#CCCCCC"), 3));
                painter.drawLine(4, 16, 28, 16);
                break;
        }
        painter.end();

        icon.addPixmap(pixmap);

        // 设置图标和文字颜色
        tabBar()->setTabIcon(index, icon);
        tabBar()->setTabTextColor(index, textColor);
    }
};
