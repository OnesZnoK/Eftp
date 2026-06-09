#pragma once

#include <QDialog>
#include "ui_SettingsDialog.h"

/**
 * @brief 设置对话框
 *
 * 功能：
 *   - 展示当前配置（环境、网络、下载）
 *   - 编辑 WiFi 配置
 *   - 切换环境
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onAddRouteClicked();
    void onDeleteRouteClicked();

private:
    void loadCurrentConfig();
    void saveWifiConfig();
    void saveEnvironment();

    Ui::SettingsDialog ui;
    QString m_currentEnv;
};
