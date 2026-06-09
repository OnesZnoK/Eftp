#pragma once

#include <QWidget>
#include "ui_DeviceInfoPanel.h"
#include "EftpTypes.h"

/**
 * @brief 设备信息展示面板
 *
 * 显示：SN、MAC、机型、区域、工序、工单、当前SKU、目标SKU、备注
 */
class DeviceInfoPanel : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceInfoPanel(QWidget* parent = nullptr);
    ~DeviceInfoPanel();

    void setDeviceInfo(const DeviceInfo& info);
    void setServerInfo(const DeviceBaseDataInfo& serverInfo);

private:
    Ui::DeviceInfoPanel ui;
};
