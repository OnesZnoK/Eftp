#pragma once

#include "ITrayAction.h"

/**
 * @brief 设备绑定动作
 *
 * 托盘菜单点击后发射 openBindDialog 信号，
 * 由 MainWindow 打开 SNMacBindDialog。
 */
class BindDeviceAction : public ITrayAction
{
    Q_OBJECT
public:
    explicit BindDeviceAction(QObject* parent = nullptr) : ITrayAction(parent) {}

    QString name() const override { return "设备绑定"; }
    bool needsSeparator() const override { return false; }

    void execute() override { emit openBindDialog(); }

signals:
    void openBindDialog();
};
