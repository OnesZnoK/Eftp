#pragma once

/**
 * @file SerApiModel.h
 * @brief EFTP 服务端 REST API 封装模块
 *
 * 功能：
 *   统一管理所有 EFTP 服务端接口调用（共 19 个）。
 *   内部自动处理：URL 拼接（从 ConfigManager 取）、HTTP 请求（调 HttpClient）、
 *   JSON 解析、日志记录。调用方只管传参拿结果。
 *
 * 业务逻辑：
 *   1. 所有 GET 请求使用 doGet（无限重试，3秒间隔）
 *   2. 所有 POST 请求使用 doPost（无限重试，3秒间隔）
 *   3. 测试项执行类接口（cycleItemStart/cycleItemEnd）使用 POST + 查询参数
 *   4. 版本检查接口（checkTestItemVersion）使用 GET
 *   5. 文件上传接口（uploadFile）使用 POST + multipart/form-data
 *
 * 接口分类：
 *   - 设备信息：queryDeviceInfo / queryDeviceRouteInfo / addMacAddr
 *   - 测试计划：queryDeviceTestInfo / queryDeviceTestInfoForWareHouse / judgeCurrentCycleEnd
 *   - 阶段生命周期：initStartOrEnd / cycleStageStartOrEnd
 *   - 测试项执行：checkTestItemVersion / cycleItemStart / cycleItemEnd /
 *                 queryCycleTestItemInfo / cycleItemReTest / queryTestItemParam
 *   - 数据上报：reportCycleTestData / addTestLog / addExtData
 *   - 文件操作：uploadFile（multipart/form-data）
 *   - 周期管理：closeDeviceCycle
 *
 * 使用方式：
 * @code
 *   SerApiModel api;
 *
 *   // 查询设备信息
 *   DeviceBaseDataInfo info = api.queryDeviceInfo("ABC123");
 *   info.skuCode / info.spuName / info.workOrderNo ...
 *
 *   // 查询测试计划
 *   TestPlanInfo plan = api.queryDeviceTestInfo("ABC123");
 *   plan.cycleId / plan.stages / plan.isInit ...
 *
 *   // 版本校验（判断是否需要下载）
 *   VersionInfo ver = api.checkTestItemVersion(itemId, localVersion);
 *   if (ver.needUpdate) {
 *       // ver.downloadUrl / ver.extractPath / ver.callHref
 *   }
 *
 *   // 测试项执行
 *   api.cycleItemStart(cycleItemId);
 *   api.cycleItemEnd(cycleItemId);
 *
 *   // 文件上传
 *   std::string url = api.uploadFile("C:/dump/crash.7z");
 * @endcode
 *
 * 依赖模块：ConfigManager（URL配置）、HttpClient（HTTP请求）、QtLogger（日志）、EftpModels（数据结构）
 */

#include <string>
#include <vector>
#include <functional>

#include "EftpTypes.h"
#include "HttpClient.h"
#include "json.hpp"

#ifdef SERAPIMODEL_LIBRARY
#  define SERAPIMODEL_EXPORT Q_DECL_EXPORT
#else
#  define SERAPIMODEL_EXPORT Q_DECL_IMPORT
#endif

using json = nlohmann::json;

/**
 * @brief SER API 封装模块
 *        统一管理所有 EFTP 服务端接口调用
 */
class SERAPIMODEL_EXPORT SerApiModel
{
public:
    SerApiModel();
    ~SerApiModel();

    /** @brief 获取最后一次 API 失败的 message */
    std::string lastError() const { return m_lastError; }

    /** @brief 设置全局 HTTP 重试回调（所有 SerApiModel 实例共享） */
    static void setRetryCallback(RetryCallback callback) { s_retryCallback = callback; }

    // ── 查询类 ──

    /**
     * @brief 查询设备基础信息
     * @param sn 设备序列号
     */
    DeviceBaseDataInfo queryDeviceInfo(const std::string& sn,
                                      const std::string& mac = "",
                                      const std::string& mainBoardSn = "");

    /**
     * @brief 查询设备工序路由
     * @param sn 设备序列号
     * @param mac MAC地址
     */
    DeviceRouteDataInfo queryDeviceRouteInfo(const std::string& sn,
                                             const std::string& mac = "");

    /**
     * @brief 查询设备测试计划（标准模式）
     * @param sn 设备序列号
     */
    TestPlanInfo queryDeviceTestInfo(const std::string& sn);

    /**
     * @brief 查询设备测试计划（仓库模式）
     * @param sn 设备序列号
     * @param mac MAC地址
     */
    TestPlanInfo queryDeviceTestInfoForWareHouse(const std::string& sn, const std::string& mac);

    /**
     * @brief 查询测试项结果
     * @param cycleItemId 测试项ID
     */
    TestItemResult queryCycleTestItemInfo(int cycleItemId);

    /**
     * @brief 查询测试项详情（含规则结果列表，用于详情对话框）
     * @param cycleItemId 测试项ID
     */
    TestDeviceCycleItemVO queryCycleTestItemDetail(int cycleItemId);

    // ── 上报类 ──

    /**
     * @brief 阶段开始或结束上报
     * @param cycleStageId 阶段ID
     * @param flag "start" 或 "end"
     */
    bool cycleStageStartOrEnd(int cycleStageId, const std::string& flag);

    /**
     * @brief 初始化开始或结束上报
     * @param technologyId 测试工艺ID
     * @param flag "start" 或 "end"
     */
    bool initStartOrEnd(int technologyId, const std::string& flag);

    /**
     * @brief 测试项开始
     * @param cycleItemId 测试项ID
     */
    bool cycleItemStart(int cycleItemId);

    /**
     * @brief 测试项结束
     * @param cycleItemId 测试项ID
     */
    bool cycleItemEnd(int cycleItemId);

    /**
     * @brief 请求重测
     * @param deviceCycleItemId 设备生命周期测试项ID
     */
    bool cycleItemReTest(int deviceCycleItemId);

    /**
     * @brief 绑定 SN 和 MAC
     * @param sn 设备序列号
     * @param mac MAC地址
     * @param mainBoardSn 主板序列号
     * @param type 类型（1或2）
     */
    bool addMacAddr(const std::string& sn, const std::string& mac,
                    const std::string& mainBoardSn, int type);

    /**
     * @brief 检查测试项版本是否需要更新
     * @param cycleItemId 测试项ID
     * @param localVersion 本地当前版本号
     */
    VersionInfo checkTestItemVersion(int cycleItemId, const std::string& localVersion);

    /**
     * @brief 上报测试周期数据
     * @param cycleItemId 测试项ID
     * @param key 数据key
     * @param testInfo 测试信息
     * @param value 数据值
     */
    bool reportCycleTestData(int cycleItemId, const std::string& key,
                             const std::string& testInfo, const json& value);

    /**
     * @brief 关闭设备测试周期
     * @param sn 设备序列号
     */
    bool closeDeviceCycle(const std::string& sn);

    /**
     * @brief 判断当前测试周期是否结束
     * @param sn 设备序列号
     * @param mac MAC地址
     */
    bool judgeCurrentCycleEnd(const std::string& sn, const std::string& mac);

    /**
     * @brief 查询测试项参数
     * @param itemId 测试项ID
     */
    json queryTestItemParam(int itemId);

    /**
     * @brief 添加测试日志
     * @param sn 设备序列号
     * @param status 状态：0=开始, 1=关闭
     */
    bool addTestLog(const std::string& sn, int status);

    /**
     * @brief 添加扩展数据（Dump文件元数据）
     * @param sn 设备序列号
     * @param mac MAC地址
     * @param fileUrl 文件URL（uploadFile返回）
     * @param dataTime 数据时间
     * @param uploadTime 上传时间
     * @param type 类型
     */
    bool addExtData(const std::string& sn, const std::string& mac,
                    const std::string& fileUrl, const std::string& dataTime,
                    const std::string& uploadTime, int type);

    /**
     * @brief 上传文件（multipart/form-data）
     * @param filePath 本地文件路径
     * @return 上传成功返回文件URL，失败返回空串
     */
    std::string uploadFile(const std::string& filePath);

private:
    ApiResponse doGet(const std::string& apiKey,
                      const std::vector<std::pair<std::string, std::string>>& params = {});
    ApiResponse doPost(const std::string& apiKey, const json& body);
    ApiResponse parseResponse(const std::string& body);
    std::string buildQueryString(
        const std::vector<std::pair<std::string, std::string>>& params);

    std::string m_lastError;  ///< 最后一次 API 失败的 message
    static RetryCallback s_retryCallback;  ///< 全局 HTTP 重试回调
};
