#pragma once
/**
 * @file EftpJsonParser.h
 * @brief JSON 解析工具（静态方法集合）
 *
 * 将 EFTP 服务端返回的 JSON 字符串解析为强类型 C++ 结构体。
 * 所有方法均为静态方法，无实例状态。
 *
 * 依赖：EftpTypes.h（数据结构定义）
 */

#include <QObject>
#include "EftpTypes.h"

class EftpJsonParser : public QObject
{
    Q_OBJECT

public:
    EftpJsonParser(QObject* parent = nullptr);
    ~EftpJsonParser();

    // ── API 响应解析（std::string 版，用于 SerApiModel）──

    /**
     * @brief 解析设备基础信息
     * @param jsonString JSON 响应字符串
     * @return DeviceBaseDataInfo 结构体
     */
    static DeviceBaseDataInfo parseDeviceBaseInfo(const QString& jsonString);

    /**
     * @brief 解析设备工序路由信息
     * @param jsonString JSON 响应字符串
     * @return DeviceRouteDataInfo 结构体
     */
    static DeviceRouteDataInfo parseDeviceRouteInfo(const QString& jsonString);

    /**
     * @brief 解析测试项版本信息
     * @param jsonString JSON 响应字符串
     * @return VersionDataInfo 结构体（UI版，包含完整字段）
     */
    static VersionDataInfo parseTestItemVersionInfo(const QString& jsonString);

    // ── 测试计划解析（UI版，用于 Widget 层）──

    static TestDeviceCycleItemDataVO parseTestDeviceCycleItemDataVO(const QJsonObject& obj);
    static TestDeviceCycleItemResultVO parseTestDeviceCycleItemResultVO(const QJsonObject& obj);
    static TestDeviceCycleItemVO parseTestDeviceCycleItemVO(const QString& jsonString);
    static TestDeviceCycleStageVO parseTestDeviceCycleStageVO(const QJsonObject& obj);
    static TestItemVO parseTestItemVO(const QJsonObject& obj);
    static TestDeviceInitInfoVO parseTestDeviceInitInfoVO(const QJsonObject& obj);
    static TestData parseTestData(const QJsonObject& obj);

    /**
     * @brief 解析完整测试计划信息
     * @param jsonString JSON 响应字符串
     * @return ComputerTestInfo 结构体
     */
    static ComputerTestInfo parseComputerTestInfo(const QString& jsonString);

    // ── 工具方法 ──

    /**
     * @brief 输出日志到文件
     * @param errorString 日志内容
     */
    static void exportLogs(const QString& errorString);
};
