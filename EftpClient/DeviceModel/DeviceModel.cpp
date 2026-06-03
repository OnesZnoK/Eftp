#include "DeviceModel.h"
#include <QProcess>
#include <QNetworkInterface>
#include <QtConcurrent>

DeviceModel::DeviceModel(QObject* parent)
    : QObject(parent)
{
}

DeviceModel::~DeviceModel()
{
}

void DeviceModel::collectAsync()
{
    QtConcurrent::run([this]() {
        DeviceInfo info = doCollect();

        if (info.deviceSn.isEmpty() && info.deviceMac.isEmpty() && info.baseboardSn.isEmpty()) {
            emit deviceInfoError("采集设备信息失败：SN、MAC、主板SN均为空");
        } else {
            emit deviceInfoReady(info);
        }
    });
}

DeviceInfo DeviceModel::doCollect()
{
    DeviceInfo info;

    // 1. 获取设备 SN（BIOS 序列号）
    info.deviceSn = executePowerShell(
        "Get-WmiObject Win32_BIOS | Select-Object -ExpandProperty SerialNumber"
    );
    info.deviceSn.replace("/", "");

    // 2. 获取主板 SN
    info.baseboardSn = executePowerShell(
        "Get-WmiObject Win32_BaseBoard | Select-Object -ExpandProperty SerialNumber"
    );
    info.baseboardSn.replace("/", "");

    // 3. 获取有线网卡 MAC 地址
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& iface : interfaces) {
        if (iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            continue;
        }
        if (iface.type() == QNetworkInterface::Ethernet) {
            QString mac = iface.hardwareAddress();
            QString description = iface.humanReadableName();

            // 去掉分隔符
            mac.remove(' ').remove(':').remove('-');

            // 跳过虚拟网卡/VPN 网卡，只取"以太网"
            if (description != QString::fromLocal8Bit("以太网")) {
                continue;
            }

            info.deviceMac = mac;
            break;
        }
    }

    return info;
}

QString DeviceModel::executePowerShell(const QString& command)
{
    QProcess process;
    process.start("powershell", QStringList() << "-Command" << command);
    process.waitForFinished(-1);
    return QString::fromLocal8Bit(process.readAllStandardOutput().trimmed());
}
