#pragma once

/**
 * @file DeviceModel.h
 * @brief 设备硬件信息采集模块
 *
 * 功能：
 *   通过 PowerShell WMI 查询采集设备的 BIOS 序列号、主板序列号、
 *   有线网卡 MAC 地址。后台线程异步执行，完成后通过信号通知。
 *
 * 采集内容：
 *   - deviceSn     — 设备序列号（BIOS SN）
 *   - deviceMac    — 有线网卡 MAC 地址（去分隔符，仅取"以太网"网卡）
 *   - baseboardSn  — 主板序列号
 *
 * 业务逻辑：
 *   1. collectAsync() 在后台线程执行采集
 *   2. 通过 PowerShell WMI 查询 BIOS 和主板序列号
 *   3. 通过 PowerShell Get-NetAdapter 获取"以太网"网卡 MAC
 *   4. 采集成功 → 发射 deviceInfoReady 信号
 *   5. 采集失败 → 发射 deviceInfoError 信号
 *
 * 使用方式：
 * @code
 *   DeviceModel* model = new DeviceModel(this);
 *   connect(model, &DeviceModel::deviceInfoReady, [](const DeviceInfo& info) {
 *       // info.deviceSn / info.deviceMac / info.baseboardSn
 *   });
 *   connect(model, &DeviceModel::deviceInfoError, [](const QString& err) {
 *       // 采集失败处理
 *   });
 *   model->collectAsync();
 * @endcode
 *
 * 依赖模块：QtLogger（日志）、EftpModels（DeviceInfo 结构体）
 * 被依赖方：MainWindow（调度器）、SerApiModel（接口参数）
 */

#include <QObject>
#include <QString>
#include "EftpTypes.h"

#ifdef DEVICEMODEL_LIBRARY
#  define DEVICEMODEL_EXPORT Q_DECL_EXPORT
#else
#  define DEVICEMODEL_EXPORT Q_DECL_IMPORT
#endif

/**
 * @brief 设备信息采集模块
 *        后台线程异步采集 SN、主板 SN、MAC 地址，完成后通过信号通知
 */
class DEVICEMODEL_EXPORT DeviceModel : public QObject
{
    Q_OBJECT

public:
    explicit DeviceModel(QObject* parent = nullptr);
    ~DeviceModel();

    /**
     * @brief 异步采集设备信息（后台线程）
     *        采集完成发射 deviceInfoReady，失败发射 deviceInfoError
     */
    void collectAsync();

    QString executePowerShell(const QString& command);

signals:
    /**
     * @brief 设备信息采集完成
     */
    void deviceInfoReady(const DeviceInfo& info);

    /**
     * @brief 设备信息采集失败
     */
    void deviceInfoError(const QString& errorMsg);

private:
    DeviceInfo doCollect();
};
