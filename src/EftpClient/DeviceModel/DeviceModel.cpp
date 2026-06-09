#include "DeviceModel.h"
#include "qtlogger.h"
#include <QProcess>

DeviceModel::DeviceModel(QObject* parent)
    : QObject(parent)
{
}

DeviceModel::~DeviceModel()
{
}

void DeviceModel::collectAsync()
{
    QtLogger::WriteLog("DeviceModel: 开始采集设备信息");

    // 直接在主线程执行（QProcess 在后台线程可能卡住）
    DeviceInfo info = doCollect();

    if (info.deviceSn.isEmpty() && info.deviceMac.isEmpty() && info.baseboardSn.isEmpty()) {
        QtLogger::WriteLog("DeviceModel: 采集失败，所有字段为空", enLogType::WARNING);
        emit deviceInfoError("采集设备信息失败：SN、MAC、主板SN均为空");
    } else {
        QtLogger::WriteLog("DeviceModel: 采集成功 SN=" + info.deviceSn + " MAC=" + info.deviceMac);
        emit deviceInfoReady(info);
    }
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

    // 3. 获取有线网卡 MAC 地址（通过 PowerShell Get-NetAdapter）
    QString macOutput = executePowerShell(
        "Get-NetAdapter -Physical | Where-Object {$_.Name -eq '以太网'} | Select-Object -ExpandProperty MacAddress"
    );
    // MAC 格式: 48-F3-17-1B-AD-89 → 48F3171BAD89
    info.deviceMac = macOutput.remove('-').remove(' ').trimmed();

    return info;
}
 
QString DeviceModel::executePowerShell(const QString& command)
{
    QtLogger::WriteLog("DeviceModel: 执行 PowerShell: " + command);
    QProcess process;
    process.start("powershell", QStringList() << "-Command" << command);

    if (!process.waitForFinished(15000)) { // 15秒超时
        QtLogger::WriteLog("DeviceModel: PowerShell 超时，强制终止", enLogType::WARNING);
        process.kill();
        return "";
    }

    QString output = QString::fromLocal8Bit(process.readAllStandardOutput().trimmed());
    QtLogger::WriteLog("DeviceModel: PowerShell 输出: " + output);
    return output;
}
