#include "SerApiModel.h"
#include "ConfigManager.h"
#include "HttpClient.h"
#include "qtlogger.h"

// 静态成员初始化
RetryCallback SerApiModel::s_retryCallback = nullptr;

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
    QtLogger::WriteLog("SerApiModel::doGet " + QString::fromStdString(apiKey));
    HttpResult res = HttpClient::GetRaw(url, 10, s_retryCallback);
    QtLogger::WriteLog(QString("SerApiModel::doGet HTTP完成, status=%1, bodySize=%2")
        .arg(res.status).arg((int)res.body.size()));
    ApiResponse resp = parseResponse(res.body);
    QtLogger::WriteLog(QString("SerApiModel::doGet 解析完成, success=%1, code=%2")
        .arg(resp.success).arg(resp.code));
    return resp;
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

DeviceBaseDataInfo SerApiModel::queryDeviceInfo(const std::string& sn,
                                                const std::string& mac,
                                                const std::string& mainBoardSn)
{
    DeviceBaseDataInfo info;
    ApiResponse resp = doGet("queryDeviceInfo", {
        {"sn", sn},
        {"mac", mac},
        {"mainBoardSn", mainBoardSn}
    });
    if (!resp.success || !resp.data.is_object()) {
        info.errorMessage = resp.message;
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

DeviceRouteDataInfo SerApiModel::queryDeviceRouteInfo(const std::string& sn,
                                                      const std::string& mac)
{
    DeviceRouteDataInfo info;
    ApiResponse resp = doGet("queryDeviceRouteInfo", {
        {"sn", sn},
        {"mac", mac}
    });
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
        plan.errorMessage = resp.message;
        QtLogger::WriteLog("queryDeviceTestInfo 失败: " + QString::fromStdString(resp.message),
                           enLogType::WARNING);
        return plan;
    }

    try {
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
                stage.isAutoExecute = stageJson.value("isAutoExecute", 0);

                // testDeviceCycleItemVOList 是二维数组 [[item, item], [item]]
                if (stageJson.contains("testDeviceCycleItemVOList") &&
                    stageJson["testDeviceCycleItemVOList"].is_array()) {
                    for (auto& groupJson : stageJson["testDeviceCycleItemVOList"]) {
                        if (groupJson.is_array()) {
                            // 内层数组：遍历每个测试项
                            for (auto& itemJson : groupJson) {
                                if (!itemJson.is_object()) continue;
                                TestItemResult item;
                                item.cycleItemId = itemJson.value("deviceCycleItemId", 0);
                                item.itemId = itemJson.value("itemId", 0);
                                item.itemName = itemJson.value("itemName", "");
                                item.testResult = itemJson.value("result", 0);
                                item.detail = itemJson.value("tips", "");
                                item.callHref = itemJson.value("callHref", "");
                                item.extractHref = itemJson.value("extractHref", "");
                                stage.items.push_back(item);
                            }
                        } else if (groupJson.is_object()) {
                            // 兼容一维数组格式
                            TestItemResult item;
                            item.cycleItemId = groupJson.value("deviceCycleItemId", 0);
                            item.itemId = groupJson.value("itemId", 0);
                            item.itemName = groupJson.value("itemName", "");
                            item.testResult = groupJson.value("result", 0);
                            item.detail = groupJson.value("tips", "");
                            item.callHref = groupJson.value("callHref", "");
                            item.extractHref = groupJson.value("extractHref", "");
                            stage.items.push_back(item);
                        }
                    }
                }
                plan.stages.push_back(stage);
            }
        }

        // 解析初始化信息（字段可能是 null，需要逐个检查）
        if (d.contains("testDeviceInitInfoVO") && d["testDeviceInitInfoVO"].is_object()) {
            auto initJson = d["testDeviceInitInfoVO"]; // 拷贝，避免引用悬挂
            if (initJson.contains("dirWhiteList") && initJson["dirWhiteList"].is_string()) {
                plan.initInfo.dirWhiteList = QString::fromStdString(initJson["dirWhiteList"].get<std::string>());
            }
            if (initJson.contains("testItemVOList") && initJson["testItemVOList"].is_array()) {
                for (auto& itemJson : initJson["testItemVOList"]) {
                    if (!itemJson.is_object()) continue;
                    TestItemVO item;
                    item.id = itemJson.value("id", 0);
                    auto safeStr = [&](const std::string& key) -> QString {
                        return (itemJson.contains(key) && itemJson[key].is_string())
                            ? QString::fromStdString(itemJson[key].get<std::string>()) : QString();
                    };
                    item.itemName = safeStr("itemName");
                    item.checkVersionResult = safeStr("checkVersionResult");
                    item.downLoadPath = safeStr("downLoadPath");
                    item.unzipPath = safeStr("unzipPath");
                    plan.initInfo.testItemVOList.append(item);
                }
            }
            QtLogger::WriteLog(QString("queryDeviceTestInfo initItems=%1").arg(plan.initInfo.testItemVOList.size()));
        } else {
            QtLogger::WriteLog("queryDeviceTestInfo: 无 testDeviceInitInfoVO");
        }
    } catch (const std::exception& e) {
        QtLogger::WriteLog(QString("queryDeviceTestInfo 解析异常: %1").arg(e.what()), enLogType::WARNING);
    }

    QtLogger::WriteLog(QString("queryDeviceTestInfo 完成, stages=%1").arg((int)plan.stages.size()));
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

TestDeviceCycleItemVO SerApiModel::queryCycleTestItemDetail(int cycleItemId)
{
    TestDeviceCycleItemVO vo;
    ApiResponse resp = doGet("queryCycleTestItemInfo", {
        {"cycleItemId", std::to_string(cycleItemId)}
    });
    if (!resp.success || !resp.data.is_object()) {
        return vo;
    }
    auto& d = resp.data;
    vo.deviceCycleItemId = d.value("deviceCycleItemId", 0);
    vo.itemName = QString::fromStdString(d.value("itemName", ""));
    vo.result = d.value("result", 0);
    vo.tips = QString::fromStdString(d.value("tips", ""));
    vo.isRepeatTest = d.value("isRepeatTest", 0);
    if (d.contains("testDeviceCycleItemResultVOList") &&
        d["testDeviceCycleItemResultVOList"].is_array()) {
        for (auto& rule : d["testDeviceCycleItemResultVOList"]) {
            TestDeviceCycleItemResultVO ruleVO;
            ruleVO.errorInfo = QString::fromStdString(rule.value("errorInfo", ""));
            ruleVO.result = rule.value("result", 1);
            ruleVO.ruleName = QString::fromStdString(rule.value("ruleName", ""));
            ruleVO.testStandard = QString::fromStdString(rule.value("testStandard", ""));
            vo.testDeviceCycleItemResultVOList.append(ruleVO);
        }
    }
    return vo;
}

// ── 上报类 ──

bool SerApiModel::cycleStageStartOrEnd(int cycleStageId, const std::string& flag)
{
    // POST + URL 查询参数
    std::string url = ConfigManager::instance().buildUrl("cycleStageStartOrEnd")
        + "?cycleStageId=" + std::to_string(cycleStageId) + "&flag=" + flag;
    HttpResult res = HttpClient::PostRawJson(url, "", 10);
    return res.success;
}

bool SerApiModel::initStartOrEnd(int technologyId, const std::string& flag)
{
    // POST + URL 查询参数
    std::string url = ConfigManager::instance().buildUrl("initStartOrEnd")
        + "?technologyId=" + std::to_string(technologyId) + "&flag=" + flag;
    HttpResult res = HttpClient::PostRawJson(url, "", 10);
    return res.success;
}

bool SerApiModel::cycleItemStart(int cycleItemId)
{
    // POST + URL 查询参数
    std::string url = ConfigManager::instance().buildUrl("cycleItemStart")
        + "?cycleItemId=" + std::to_string(cycleItemId) + "&flag=start";
    HttpResult res = HttpClient::PostRawJson(url, "", 10);
    return res.success;
}

bool SerApiModel::cycleItemEnd(int cycleItemId)
{
    // POST + URL 查询参数
    std::string url = ConfigManager::instance().buildUrl("cycleItemEnd")
        + "?cycleItemId=" + std::to_string(cycleItemId) + "&flag=end";
    HttpResult res = HttpClient::PostRawJson(url, "", 10);
    return res.success;
}

bool SerApiModel::cycleItemReTest(int deviceCycleItemId)
{
    // POST + 查询参数
    std::string url = ConfigManager::instance().buildUrl("cycleItemReTest")
        + "?cycleItemId=" + std::to_string(deviceCycleItemId);
    HttpResult res = HttpClient::PostRawJson(url, "", 10);
    return res.success;
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

    if (!resp.success) {
        QtLogger::WriteLog("checkTestItemVersion 失败: " + QString::fromStdString(resp.message), enLogType::WARNING);
        return info;
    }

    try {
        json d = resp.data;  // 拷贝而非引用，避免悬挂
        if (!d.is_object()) {
            QtLogger::WriteLog("checkTestItemVersion: data 不是对象");
            return info;
        }

        // 安全提取字段
        info.downloadUrl = d.value("downLoadPath", std::string(""));
        info.extractPath = d.value("extractHref", std::string(""));
        info.callHref = d.value("callHref", std::string(""));
        info.itemName = d.value("itemName", std::string(""));
        info.programName = d.value("programName", std::string(""));
        info.programType = d.value("programType", 0);
        info.isAutoExecute = (d.value("isAutoExecute", 0) == 1);
        info.isRepeatTest = (d.value("isRepeatTest", 0) == 1);
        info.tips = d.value("tips", std::string(""));
        info.version = d.value("version", std::string(""));

        // 版本比较：服务端版本与本地版本不一致时需要下载
        info.needUpdate = (info.version != localVersion) && !info.version.empty();

        QtLogger::WriteLog(QString("checkTestItemVersion 解析完成, serverVer=%1, localVer=%2, needUpdate=%3, callHref=%4")
            .arg(QString::fromStdString(info.version))
            .arg(QString::fromStdString(localVersion))
            .arg(info.needUpdate)
            .arg(QString::fromStdString(info.callHref)));
    } catch (const std::exception& e) {
        QtLogger::WriteLog(QString("checkTestItemVersion 异常: %1").arg(e.what()), enLogType::WARNING);
    }

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
    // POST + 查询参数
    std::string url = ConfigManager::instance().buildUrl("addTestLog")
        + "?sn=" + sn + "&status=" + std::to_string(status);
    HttpResult res = HttpClient::PostRawJson(url, "", 10);
    return res.success;
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
        m_lastError = resp.message;
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
            stage.isAutoExecute = stageJson.value("isAutoExecute", 0);

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
    HttpResult res = HttpClient::PostRawJson(url, "{}", 60);

    // TODO: 使用 DownloadManager::uploadFile 替代
    // 暂时返回空串，后续通过信号路由到 DownloadManager
    Q_UNUSED(filePath);
    return "";
}
