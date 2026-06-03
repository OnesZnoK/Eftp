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
 * 依赖模块：ConfigManager（URL配置）、HttpCilent（HTTP请求）、QtLogger（日志）、JsonHpp（JSON解析）
 */

#include <string>
#include <vector>
#include <functional>

#include "json.hpp"

#ifdef SERAPIMODEL_LIBRARY
#  define SERAPIMODEL_EXPORT __declspec(dllexport)
#else
#  define SERAPIMODEL_EXPORT __declspec(dllimport)
#endif

using json = nlohmann::json;

/**
 * @brief 通用 API 响应
 */
struct SERAPIMODEL_EXPORT ApiResponse {
    bool success = false;           // 请求是否成功（code==0 且 HTTP 2xx）
    int code = -1;                  // 服务端业务状态码（0=成功, 1001=未绑定, 1003=绑定不匹配）
    std::string message;            // 服务端返回的消息
    json data;                      // 服务端返回的 data 字段
    std::string rawBody;            // 完整的 HTTP 响应体原文
};

/**
 * @brief 设备基础信息（来自 queryDeviceInfo）
 */
struct SERAPIMODEL_EXPORT DeviceBaseDataInfo {
    std::string sn;                 // 设备序列号
    std::string skuCode;            // SKU编码
    std::string skuName;            // SKU名称
    std::string spuName;            // 机型
    std::string areaName;           // 工厂区域
    std::string routeProcessesName; // 当前工序名称
    std::string workOrderNo;        // 工单号
    std::string targetSku;          // 目标SKU
    std::string targetSkuName;      // 目标SKU名称
    std::string isStandard;         // 是否标准品
    std::string remark;             // 备注
};

/**
 * @brief 工序路由信息（来自 queryDeviceRouteInfo）
 */
struct SERAPIMODEL_EXPORT DeviceRouteDataInfo {
    std::string sn;                 // 设备序列号
    int routeId = 0;                // 工艺路线ID
    std::string routeName;          // 工艺路线名称
    int routeProcessesId = 0;       // 工序ID（用于WiFi路由映射）
    std::string routeProcessesName; // 工序名称
    int orderType = 0;              // 工单类型
    std::string orderTypeName;      // 工单类型名称
    int productType = 0;            // 产品类别
    int area = 0;                   // 工厂区域
    std::string workOrderNo;        // 工单号
    int workOrderType = 0;          // 工单类型
    std::string workOrderTypeName;  // 工单类型名称
};

/**
 * @brief 测试项结果
 */
struct SERAPIMODEL_EXPORT TestItemResult {
    int cycleItemId = 0;            // 测试项ID
    std::string itemName;           // 测试项名称
    int testResult = 0;             // 测试结果：0=无, 1=通过, 2=失败
    std::string failReason;         // 失败原因
    std::string detail;             // 详细信息
};

/**
 * @brief 测试阶段
 */
struct SERAPIMODEL_EXPORT TestStageInfo {
    int stageId = 0;                // 阶段ID
    std::string stageName;          // 阶段名称
    int stageState = 0;             // 阶段状态：0=未开始, 1=运行中, 2=完成
    std::vector<TestItemResult> items;  // 该阶段下的所有测试项
};

/**
 * @brief 完整测试计划（来自 queryDeviceTestInfo）
 */
struct SERAPIMODEL_EXPORT TestPlanInfo {
    int cycleId = 0;                // 测试周期ID
    int technologyId = 0;           // 测试工艺ID
    int isInit = 0;                 // 是否需要初始化：0=不需要, 1=需要
    int initStatus = 0;             // 初始化状态：0=未开始, 1=进行中, 2=已完成
    int isAutoExecute = 0;          // 是否自动执行：0=否, 1=是
    int status = 0;                 // 状态：0=未开始, 1=测试中, 2=已完成, 3=已关闭
    std::string sn;                 // 设备序列号
    std::vector<TestStageInfo> stages;  // 所有测试阶段列表
};

/**
 * @brief 测试项版本信息（来自 checkTestItemVersion）
 */
struct SERAPIMODEL_EXPORT VersionInfo {
    bool needUpdate = false;        // 是否需要更新（checkVersionResult=="0" 时为 true）
    std::string downloadUrl;        // 程序包下载路径（downLoadPath）
    std::string extractPath;        // 解压路径（extractHref）
    std::string callHref;           // 程序入口路径
    std::string itemName;           // 测试项名称
    std::string programName;        // 程序名称
    int programType = 0;            // 程序类型：1=exe, 2=bat
    bool isAutoExecute = false;     // 是否自动执行
    bool isRepeatTest = false;      // 是否支持重测
    std::string tips;               // 知识库提示
    std::string md5;                // 【预留】文件 MD5 校验值（服务端暂未实现）
};

/**
 * @brief SER API 封装模块
 *        统一管理所有 EFTP 服务端接口调用
 */
class SERAPIMODEL_EXPORT SerApiModel
{
public:
    SerApiModel();
    ~SerApiModel();

    // ── 查询类 ──

    /**
     * @brief 查询设备基础信息
     * @param sn 设备序列号
     */
    DeviceBaseDataInfo queryDeviceInfo(const std::string& sn);

    /**
     * @brief 查询设备工序路由
     * @param sn 设备序列号
     */
    DeviceRouteDataInfo queryDeviceRouteInfo(const std::string& sn);

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
};
