#pragma once

#include <QObject>
#include <QString>

/**
 * @brief 托盘菜单动作接口
 *
 * 所有托盘菜单项继承此接口，SystemTrayManager 自动注册到右键菜单。
 * 后续扩展只需新增子类，无需修改 SystemTrayManager。
 */
class ITrayAction : public QObject
{
    Q_OBJECT
public:
    explicit ITrayAction(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~ITrayAction() = default;

    /** @brief 菜单显示文字 */
    virtual QString name() const = 0;

    /** @brief 是否需要分隔线（在该项之前） */
    virtual bool needsSeparator() const { return false; }

public slots:
    /** @brief 点击菜单项时执行 */
    virtual void execute() = 0;
};
