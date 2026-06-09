#pragma once

#include "ITrayAction.h"

/**
 * @brief 设置对话框动作
 *
 * 点击后发射 openSettings 信号，由 MainWindow 打开设置对话框。
 * 设置对话框是 UI 层组件，通过信号解耦。
 */
class SettingsAction : public ITrayAction
{
    Q_OBJECT
public:
    explicit SettingsAction(QObject* parent = nullptr) : ITrayAction(parent) {}

    QString name() const override { return "设置"; }
    bool needsSeparator() const override { return true; }

    void execute() override { emit openSettings(); }

signals:
    void openSettings();
};
