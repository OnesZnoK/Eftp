#pragma once

#include <QTabBar>
#include <QPainter>
#include <QMap>
#include <QMouseEvent>
#include "EftpTypes.h"

/**
 * @brief 自定义 TabBar — 每个 Tab 显示状态图标（未开始/运行中/成功/失败）
 *        未开始的 Tab 不可点击，Tab 之间有虚线连接
 */
class EDYTabBar : public QTabBar
{
    Q_OBJECT

public:
    explicit EDYTabBar(QWidget* parent);
    ~EDYTabBar();

    void setTabStatus(int index, TestState status);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    QSize sizeHint() const override;
    QSize tabSizeHint(int index) const override;

private:
    QMap<int, TestState> m_tabStatus;
};
