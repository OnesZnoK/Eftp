#include "SerApiModel.h"
#include "ConfigManager.h"
#include "HttpClient.h"
#include "qtlogger.h"

SerApiModel::SerApiModel()
{
}

SerApiModel::~SerApiModel()
{
}

// ── 内部请求方法

ApiResponse SerApiModel::doGet(const std::string& apiKey,
                                const std::vector<std::pair<std::string, std::string>>& params)
{
    std::string url = ConfigManager::instance().buildUrl(apiKey);
    if (!params.empty()) {
        url += "?" + buildQueryString(params);
    }
    HttpResult res = HttpClient::GetRaw(url);
    return parseResponse(res.body);
}

ApiResponse SerApiModel::doPost(const std::string& apiKey, const json& body)
{
    std::string url = ConfigManager::instance().buildUrl(apiKey);
    int timeoutSec = ConfigManager::instance().downloadTimeoutMs() / 1000;
    HttpResult res = HttpClient::PostRawJson(url, body.dump(), timeoutSec);
    return parseResponse(res.body);
}

ApiResponse SerApiModel::parseResponse(const std::string& body)
{
    ApiResponse resp;
    resp.rawBody = body;
    try {
        json j = json::parse(body);
        resp.code = j.value("code", -1);
        resp.message = j.value("message", "");
        resp.success = (resp.code == 0);
        if (j.contains("data")) {
            resp.data = j["data"];
        }
    } catch (const json::parse_error& e) {
        resp.success = false;
        resp.message = std::string("JSON解析失败: ") + e.what();
    }
    return resp;
}

std::string SerApiModel::buildQueryString(
    const std::vector<std::pair<std::string, std::string>>& params)
{
    std::string qs;
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) qs += "&";
        qs += params[i].first + "=" + params[i].second;
    }
    return qs;
}

// ── 查询类 ──

DeviceBaseDataInfo SerApiModel::queryDeviceInfo(const std::string& sn)
{
    DeviceBaseDataInfo info;
    ApiResponse resp = doGet("queryDeviceInfo", {{"sn", sn}});
    if (!resp.success || !resp.data.is_object()) {
        QtLogger::WriteLog("queryDeviceInfo 失败: " + QString::fromStdString(resp.message),
                           enLogType::WARNING);
        return info;
    }
    auto& d = resp.data;
    info.sn = d.value("sn", "");
    info.skuCode = d.value("skuCode", "");
    info.skuName = d.value("skuName", "");
    info.spuName = d.value("spuName", "");
    info.areaName = d.value("areaName", "");
    info.routeProcessesName = d.value("routeProcessesName", "");
    info.workOrderNo = d.value("workOrderNo", "");
    info.targetSku = d.value("targetSku", "");
    info.targetSkuName = d.value("targetSkuName", "");
    info.isStandard = d.value("isStandard", "");
    info.remark = d.value("remark", "");
    return info;
}

DeviceRouteDataInfo SerApiModel::queryDeviceRouteInfo(const std::string& sn)
{
    DeviceRouteDataInfo info;
    ApiResponse resp = doGet("queryDeviceRouteInfo", {{"sn", sn}});
    if (!resp.success || !resp.data.is_object()) {
        QtLogger::WriteLog("queryDeviceRouteInfo 失败: " + QString::fromStdString(resp.message),
                           enLogType::WARNING);
        return info;
    }
    auto& d = resp.data;
    info.sn = d.value("sn", "");
    info.routeId = d.value("routeId", 0);
    info.routeName = d.value("routeName", "");
    info.routeProcessesId = d.value("routeProcessesId", 0);
    info.routeProcessesName = d.value("routeProcessesName", "");
    info.orderType = d.value("orderType", 0);
    info.orderTypeName = d.value("orderTypeName", "");
    info.productType = d.value("productType", 0);
    info.area = d.value("area", 0);
    info.workOrderNo = d.value("workOrderNo", "");
    info.workOrderType = d.value("workOrderType", 0);
    info.workOrderTypeName = d.value("workOrderTypeName", "");
    return info;
}

TestPlanInfo SerApiModel::queryDeviceTestInfo(const std::string& sn)
{
    TestPlanInfo plan;
    ApiResponse resp = doGet("queryDeviceTestInfo", {{"sn", sn}});
    if (!resp.success || !resp.data.is_object()) {
        QtLogger::WriteLog("queryDeviceTestInfo 失败: " + QString::fromStdString(resp.message),
                           enLogType::WARNING);
        return plan;
    }
    auto& d = resp.data;
    plan.cycleId = d.value("cycleId", 0);
    plan.technologyId = d.value("technologyId", 0);
    plan.isInit = d.value("isInit", 0);
    plan.initStatus = d.value("initStatus", 0);
    plan.isAutoExecute = d.value("isAutoExecute", 0);
    plan.status = d.value("status", 0);
    plan.sn = d.value("sn", "");

    if (d.contains("testDeviceCycleStageVOList") && d["testDeviceCycleStageVOList"].is_array()) {
        for (auto& stageJson : d["testDeviceCycleStageVOList"]) {
            TestStageInfo stage;
            stage.stageId = stageJson.value("stageId", 0);
            stage.stageName = stageJson.value("stageName", "");
            stage.stageState = stageJson.value("status", 0);

            if (stageJson.contains("testDeviceCycleItemVOList") &&
                stageJson["testDeviceCycleItemVOList"].is_array()) {
                for (auto& itemJson : stageJson["testDeviceCycleItemVOList"]) {
                    TestItemResult item;
                    item.cycleItemId = itemJson.value("deviceCycleItemId", 0);
                    item.itemName = itemJson.value("itemName", "");
                    item.testResult = itemJson.value("result", 0);
                    item.detail = itemJson.value("tips", "");
                    stage.items.push_back(item);
                }
            }
            plan.stages.push_back(stage);
        }
    }
    return plan;
}

TestItemResult SerApiModel::queryCycleTestItemInfo(int cycleItemId)
{
    TestItemResult item;
    ApiResponse resp = doGet("queryCycleTestItemInfo", {
        {"cycleItemId", std::to_string(cycleItemId)}
    });
    if (!resp.success || !resp.data.is_object()) {
        return item;
    }
    auto& d = resp.data;
    item.cycleItemId = d.value("deviceCycleItemId", 0);
    item.itemName = d.value("itemName", "");
    item.testResult = d.value("result", 0);
    item.detail = d.value("tips", "");
    if (d.contains("testDeviceCycleItemResultVOList") &&
        d["testDeviceCycleItemResultVOList"].is_array()) {
        for (auto& rule : d["testDeviceCycleItemResultVOList"]) {
            if (rule.value("result", 1) == 0) {
                item.failReason = rule.value("errorInfo", "");
                break;
            }
        }
    }
    return item;
}

// ── 上报类 ──

bool SerApiModel::cycleStageStartOrEnd(int cycleStageId, const std::string& flag)
{
    json body = {
        {"cycleStageId", cycleStageId},
        {"flag", flag}
    };
    return doPost("cycleStageStartOrEnd", body).success;
}

bool SerApiModel::initStartOrEnd(int technologyId, const std::string& flag)
{
    json body = {
        {"technologyId", technologyId},
        {"flag", flag}
    };
    return doPost("initStartOrEnd", body).success;
}

bool SerApiModel::cycleItemStart(int cycleItemId)
{
    json body = {
        {"cycleItemId", cycleItemId},
        {"flag", "start"}
    };
    return doPost("cycleItemStart", body).success;
}

bool SerApiModel::cycleItemEnd(int cycleItemId)
{
    json body = {
        {"cycleItemId", cycleItemId},
        {"flag", "end"}
    };
    return doPost("cycleItemEnd", body).success;
}

bool SerApiModel::cycleItemReTest(int deviceCycleItemId)
{
    json body = {
        {"cycleItemId", deviceCycleItemId}
    };
    return doPost("cycleItemReTest", body).success;
}

bool SerApiModel::addMacAddr(const std::string& sn, const std::string& mac,
                              const std::string& mainBoardSn, int type)
{
    json body = {
        {"sn", sn},
        {"mac", mac},
        {"mainBoardSn", mainBoardSn},
        {"type", type}
    };
    return doPost("addMacAddr", body).success;
}

VersionInfo SerApiModel::checkTestItemVersion(int cycleItemId, const std::string& localVersion)
{
    VersionInfo info;
    ApiResponse resp = doGet("checkTestItemVersion", {
        {"cycleItemId", std::to_string(cycleItemId)},
        {"version", localVersion}
    });
    if (!resp.success || !resp.data.is_object()) {
        return info;
    }
    auto& d = resp.data;
    info.needUpdate = (d.value("checkVersionResult", "1") == "0");
    info.downloadUrl = d.value("downLoadPath", "");
    info.extractPath = d.value("extractHref", "");
    info.callHref = d.value("callHref", "");
    info.itemName = d.value("itemName", "");
    info.programName = d.value("programName", "");
    info.programType = d.value("programType", 0);
    info.isAutoExecute = (d.value("isAutoExecute", 0) == 1);
    info.isRepeatTest = (d.value("isRepeatTest", 0) == 1);
    info.tips = d.value("tips", "");
    info.md5 = d.value("md5", "");  // 预留
    return info;
}

bool SerApiModel::reportCycleTestData(int cycleItemId, const std::string& key,
                                       const std::string& testInfo, const json& value)
{
    json body = {
        {"cycleItemId", cycleItemId},
        {"key", key},
        {"testInfo", testInfo},
        {"value", value}
    };
    return doPost("reportCycleTestData", body).success;
}

bool SerApiModel::closeDeviceCycle(const std::string& sn)
{
    json body = {
        {"sn", sn},
        {"flag", 1}
    };
    return doPost("closeDeviceCycle", body).success;
}

bool SerApiModel::judgeCurrentCycleEnd(const std::string& sn, const std::string& mac)
{
    json body = {
        {"sn", sn},
        {"mac", mac}
    };
    return doPost("judgeCurrentCycleEnd", body).success;
}

json SerApiModel::queryTestItemParam(int itemId)
{
    ApiResponse resp = doGet("queryTestItemParam", {
        {"itemId", std::to_string(itemId)}
    });
    if (resp.success && resp.data.is_array()) {
        return resp.data;
    }
    return json::array();
}

bool SerApiModel::addTestLog(const std::string& sn, int status)
{
    json body = {
        {"sn", sn},
        {"status", status}
    };
    return doPost("addTestLog", body).success;
}

bool SerApiModel::addExtData(const std::string& sn, const std::string& mac,
                              const std::string& fileUrl, const std::string& dataTime,
                              const std::string& uploadTime, int type)
{
    json body = {
        {"sn", sn},
        {"mac", mac},
        {"fileUrl", fileUrl},
        {"dataTime", dataTime},
        {"uploadTime", uploadTime},
        {"type", type}
    };
    return doPost("addExtData", body).success;
}

TestPlanInfo SerApiModel::queryDeviceTestInfoForWareHouse(const std::string& sn, const std::string& mac)
{
    TestPlanInfo plan;
    ApiResponse resp = doGet("queryDeviceTestInfoForWareHouse", {
        {"sn", sn},
        {"mac", mac}
    });
    if (!resp.success || !resp.data.is_object()) {
        QtLogger::WriteLog("queryDeviceTestInfoForWareHouse 失败: " + QString::fromStdString(resp.message),
                           enLogType::WARNING);
        return plan;
    }
    auto& d = resp.data;
    plan.cycleId = d.value("cycleId", 0);
    plan.technologyId = d.value("technologyId", 0);
    plan.isInit = d.value("isInit", 0);
    plan.initStatus = d.value("initStatus", 0);
    plan.isAutoExecute = d.value("isAutoExecute", 0);
    plan.status = d.value("status", 0);
    plan.sn = d.value("sn", "");

    if (d.contains("testDeviceCycleStageVOList") && d["testDeviceCycleStageVOList"].is_array()) {
        for (auto& stageJson : d["testDeviceCycleStageVOList"]) {
            TestStageInfo stage;
            stage.stageId = stageJson.value("stageId", 0);
            stage.stageName = stageJson.value("stageName", "");
            stage.stageState = stageJson.value("status", 0);

            if (stageJson.contains("testDeviceCycleItemVOList") &&
                stageJson["testDeviceCycleItemVOList"].is_array()) {
                for (auto& itemJson : stageJson["testDeviceCycleItemVOList"]) {
                    TestItemResult item;
                    item.cycleItemId = itemJson.value("deviceCycleItemId", 0);
                    item.itemName = itemJson.value("itemName", "");
                    item.testResult = itemJson.value("result", 0);
                    item.detail = itemJson.value("tips", "");
                    stage.items.push_back(item);
                }
            }
            plan.stages.push_back(stage);
        }
    }
    return plan;
}

std::string SerApiModel::uploadFile(const std::string& filePath)
{
    std::string url = ConfigManager::instance().buildUrl("uploadFile");

    HttpResult res = HttpClient::UploadFile(url, filePath);

    if (res.success) {
        ApiResponse resp = parseResponse(res.body);
        if (resp.success && resp.data.is_string()) {
            return resp.data.get<std::string>();
        }
    }
    return "";
}
