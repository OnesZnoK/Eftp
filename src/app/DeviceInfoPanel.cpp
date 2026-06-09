#include "DeviceInfoPanel.h"

DeviceInfoPanel::DeviceInfoPanel(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

DeviceInfoPanel::~DeviceInfoPanel()
{
}

void DeviceInfoPanel::setDeviceInfo(const DeviceInfo& info)
{
    ui.labelSn->setText("SN: " + info.deviceSn);
    ui.labelMac->setText("MAC: " + info.deviceMac);
}

void DeviceInfoPanel::setServerInfo(const DeviceBaseDataInfo& info)
{
    ui.labelDeviceType->setText("机型: " + QString::fromUtf8(info.spuName.c_str()));
    ui.labelArea->setText("区域: " + QString::fromUtf8(info.areaName.c_str()));
    ui.labelWorkStation->setText("工序: " + QString::fromUtf8(info.routeProcessesName.c_str()));
    ui.labelWorkOrder->setText("工单: " + QString::fromUtf8(info.workOrderNo.c_str()));
    ui.labelCurrentSKU->setText("当前SKU: " + QString::fromUtf8(info.skuCode.c_str()) + " " + QString::fromUtf8(info.skuName.c_str()));
    ui.labelAimSKU->setText("目标SKU: " + QString::fromUtf8(info.targetSku.c_str()) + " " + QString::fromUtf8(info.targetSkuName.c_str()));
    ui.labelNotes->setText("备注: " + QString::fromUtf8(info.remark.c_str()));
}
