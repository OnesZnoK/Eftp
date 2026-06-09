#pragma once

/**
 * @file SystemTrayManager.h
 * @brief 系统托盘管理器（STATIC）
 *
 * 功能：
 *   - 程序启动后在右下角显示托盘图标
 *   - 右键菜单：显示主窗口 / [actions...] / 退出
 *   - 通过 ITrayAction 接口动态注册菜单项
 */

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QList>
#include "actions/ITrayAction.h"

class SystemTrayManager : public QObject
{
    Q_OBJECT
public:
    explicit SystemTrayManager(QObject* parent = nullptr);
    ~SystemTrayManager();

    /**
     * @brief 初始化托盘图标并显示
     * @param iconPath 图标路径
     */
    void init(const QString& iconPath);

    /**
     * @brief 注册一个菜单动作（在"退出"之前）
     */
    void addAction(ITrayAction* action);

    /**
     * @brief 显示托盘气泡通知
     */
    void showMessage(const QString& title, const QString& msg);

signals:
    void showMainWindow();
    void exitApp();

private:
    QSystemTrayIcon* m_trayIcon = nullptr;
    QMenu* m_menu = nullptr;
    QList<ITrayAction*> m_actions;
};
