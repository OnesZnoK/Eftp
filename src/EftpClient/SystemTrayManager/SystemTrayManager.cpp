#include "SystemTrayManager.h"
#include <QApplication>
#include <QIcon>

SystemTrayManager::SystemTrayManager(QObject* parent)
    : QObject(parent)
{
}

SystemTrayManager::~SystemTrayManager()
{
    qDeleteAll(m_actions);
    delete m_menu;
    delete m_trayIcon;
}

void SystemTrayManager::init(const QString& iconPath)
{
    m_trayIcon = new QSystemTrayIcon(QIcon(iconPath), this);
    m_trayIcon->setToolTip("EFTP - 电子工厂测试平台");

    m_menu = new QMenu();

    // 显示主窗口
    QAction* showAction = m_menu->addAction("显示主窗口");
    connect(showAction, &QAction::triggered, this, &SystemTrayManager::showMainWindow);

    // 动态注册所有 action
    for (ITrayAction* action : m_actions) {
        if (action->needsSeparator()) {
            m_menu->addSeparator();
        }
        QAction* act = m_menu->addAction(action->name());
        connect(act, &QAction::triggered, action, &ITrayAction::execute);
    }

    // 退出
    m_menu->addSeparator();
    QAction* exitAction = m_menu->addAction("退出");
    connect(exitAction, &QAction::triggered, this, &SystemTrayManager::exitApp);

    m_trayIcon->setContextMenu(m_menu);

    // 双击显示主窗口
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            emit showMainWindow();
        }
    });

    m_trayIcon->show();
}

void SystemTrayManager::addAction(ITrayAction* action)
{
    m_actions.append(action);
}

void SystemTrayManager::showMessage(const QString& title, const QString& msg)
{
    if (m_trayIcon) {
        m_trayIcon->showMessage(title, msg, QSystemTrayIcon::Information, 3000);
    }
}
