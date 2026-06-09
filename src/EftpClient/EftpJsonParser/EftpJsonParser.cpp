#include "EftpJsonParser.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QCoreApplication>

EftpJsonParser::EftpJsonParser(QObject* parent) : QObject(parent) {}
EftpJsonParser::~EftpJsonParser() {}

// ── API 响应解析 ──

DeviceBaseDataInfo EftpJsonParser::parseDeviceBaseInfo(const QString& jsonString) {
    DeviceBaseDataInfo info;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (!doc.isObject())
        return info;

    QJsonObject obj = doc.object();
    int code = obj["code"].toInt();
    QJsonObject dataObj = obj["data"].toObject();

    info.areaName = dataObj["areaName"].toString().toStdString();
    info.isStandard = dataObj["isStandard"].toString().toStdString();
    info.remark = dataObj["remark"].toString().toStdString();
    info.routeProcessesName = dataObj["routeProcessesName"].toString().toStdString();
    info.skuCode = dataObj["skuCode"].toString().toStdString();
    info.skuName = dataObj["skuName"].toString().toStdString();
    info.spuName = dataObj["spuName"].toString().toStdString();
    info.targetSku = dataObj["targetSku"].toString().toStdString();
    info.targetSkuName = dataObj["targetSkuName"].toString().toStdString();
    info.workOrderNo = dataObj["workOrderNo"].toString().toStdString();

    if (code == 0) {
        info.sn = dataObj["sn"].toString().toStdString();
    }
    return info;
}

DeviceRouteDataInfo EftpJsonParser::parseDeviceRouteInfo(const QString& jsonString) {
    DeviceRouteDataInfo info;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (!doc.isObject())
        return info;

    QJsonObject obj = doc.object();
    QJsonObject dataObj = obj["data"].toObject();

    info.sn = dataObj["sn"].toString().toStdString();
    info.workOrderNo = dataObj["workOrderNo"].toString().toStdString();
    info.workOrderType = dataObj["workOrderType"].toInt();
    info.workOrderTypeName = dataObj["workOrderTypeName"].toString().toStdString();
    info.orderType = dataObj["orderType"].toInt();
    info.orderTypeName = dataObj["orderTypeName"].toString().toStdString();
    info.area = dataObj["area"].toInt();
    info.productType = dataObj["productType"].toInt();
    info.routeId = dataObj["routeId"].toInt();
    info.routeName = dataObj["routeName"].toString().toStdString();
    info.routeProcessesId = dataObj["routeProcessesId"].toInt();
    info.routeProcessesName = dataObj["routeProcessesName"].toString().toStdString();
    return info;
}

VersionDataInfo EftpJsonParser::parseTestItemVersionInfo(const QString& jsonString) {
    VersionDataInfo dataInfo;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonObject rootObj = doc.object();

    QJsonObject dataObj = rootObj["data"].toObject();
    dataInfo.id = dataObj["id"].toString();
    dataInfo.itemName = dataObj["itemName"].toString();
    dataInfo.productType = dataObj["productType"].toInt();
    dataInfo.productTypeDesc = dataObj["productTypeDesc"].toString();
    dataInfo.programType = dataObj["programType"].toInt();
    dataInfo.programTypeDesc = dataObj["programTypeDesc"].toString();
    dataInfo.callHref = dataObj["callHref"].toString();
    dataInfo.extractHref = dataObj["extractHref"].toString();
    dataInfo.status = dataObj["status"].toInt();
    dataInfo.statusDesc = dataObj["statusDesc"].toString();
    dataInfo.isRepeatTest = dataObj["isRepeatTest"].toBool();
    dataInfo.isOffline = dataObj["isOffline"].toBool();
    dataInfo.checkVersionResult = dataObj["checkVersionResult"].toString();
    dataInfo.downLoadPath = dataObj["downLoadPath"].toString();
    dataInfo.unzipPath = dataObj["unzipPath"].toString();
    dataInfo.version = dataObj["version"].toString();
    dataInfo.createBy = dataObj["createBy"].toString();
    dataInfo.createTime = dataObj["createTime"].toString();
    dataInfo.tips = dataObj["tips"].toString();
    dataInfo.updateBy = dataObj["updateBy"].toString();
    dataInfo.updateTime = dataObj["updateTime"].toString();

    return dataInfo;
}

// ── 测试计划解析 ──

TestDeviceCycleItemDataVO EftpJsonParser::parseTestDeviceCycleItemDataVO(const QJsonObject& obj) {
    TestDeviceCycleItemDataVO dataVO;
    dataVO.name = obj["name"].toString();
    dataVO.value = obj["value"].toString();
    return dataVO;
}

TestDeviceCycleItemResultVO EftpJsonParser::parseTestDeviceCycleItemResultVO(const QJsonObject& obj) {
    TestDeviceCycleItemResultVO resultVO;
    resultVO.errorInfo = obj["errorInfo"].toString();
    resultVO.result = obj["result"].toInt();
    resultVO.ruleName = obj["ruleName"].toString();
    resultVO.testStandard = obj["testStandard"].toString();

    QJsonArray dataList = obj["testDeviceCycleItemDataVOList"].toArray();
    for (int i = 0; i < dataList.size(); ++i) {
        resultVO.testDeviceCycleItemDataVOList.append(parseTestDeviceCycleItemDataVO(dataList[i].toObject()));
    }
    return resultVO;
}

TestDeviceCycleItemVO EftpJsonParser::parseTestDeviceCycleItemVO(const QString& jsonString) {
    TestDeviceCycleItemVO itemVO;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonObject obj = doc.object();

    itemVO.callHref = obj["callHref"].toString();
    itemVO.deviceCycleId = obj["deviceCycleId"].toInt();
    itemVO.deviceCycleStageId = obj["deviceCycleStageId"].toInt();
    itemVO.extractHref = obj["extractHref"].toString();
    itemVO.id = obj["id"].toInt();
    itemVO.deviceCycleItemId = obj["deviceCycleItemId"].toInt();
    itemVO.isOffline = obj["isOffline"].toInt();
    itemVO.isRepeatTest = obj["isRepeatTest"].toInt();
    itemVO.itemId = obj["itemId"].toInt();
    itemVO.itemName = obj["itemName"].toString();
    itemVO.productType = obj["productType"].toInt();
    itemVO.programType = obj["programType"].toInt();
    itemVO.result = obj["result"].toInt();
    itemVO.stageId = obj["stageId"].toInt();
    itemVO.status = obj["status"].toInt();
    itemVO.technologyId = obj["technologyId"].toInt();
    itemVO.tips = obj["tips"].toString();

    QJsonArray resultList = obj["testDeviceCycleItemResultVOList"].toArray();
    for (int i = 0; i < resultList.size(); ++i) {
        itemVO.testDeviceCycleItemResultVOList.append(parseTestDeviceCycleItemResultVO(resultList[i].toObject()));
    }
    return itemVO;
}

TestDeviceCycleStageVO EftpJsonParser::parseTestDeviceCycleStageVO(const QJsonObject& obj) {
    TestDeviceCycleStageVO stageVO;
    stageVO.deviceCycleId = obj["deviceCycleId"].toInt();
    stageVO.endTime = obj["endTime"].toString();
    stageVO.id = obj["id"].toInt();
    stageVO.deviceCycleStageId = obj["deviceCycleStageId"].toInt();
    stageVO.result = obj["result"].toString();
    stageVO.sn = obj["sn"].toString();
    stageVO.stageId = obj["stageId"].toInt();
    stageVO.stageName = obj["stageName"].toString();
    stageVO.startTime = obj["startTime"].toString();
    stageVO.status = obj["status"].toInt();
    stageVO.isAutoExecute = obj["isAutoExecute"].toInt() == 1;

    if (obj.contains("testDeviceCycleItemVOList") && obj["testDeviceCycleItemVOList"].isArray()) {
        QJsonArray array = obj["testDeviceCycleItemVOList"].toArray();
        for (int i = 0; i < array.size(); ++i) {
            QJsonArray innerArray = array[i].toArray();
            QList<TestDeviceCycleItemVO> itemList;
            for (int j = 0; j < innerArray.size(); ++j) {
                QJsonObject itemObj = innerArray[j].toObject();
                QJsonDocument doc(itemObj);
                QString jsonString = doc.toJson(QJsonDocument::Compact);
                TestDeviceCycleItemVO item = parseTestDeviceCycleItemVO(jsonString);
                itemList.append(item);
            }
            stageVO.testDeviceCycleItemVOList.append(itemList);
        }
    }
    return stageVO;
}

TestItemVO EftpJsonParser::parseTestItemVO(const QJsonObject& obj) {
    TestItemVO itemVO;
    itemVO.checkVersionResult = obj["checkVersionResult"].toString();
    itemVO.downLoadPath = obj["downLoadPath"].toString();
    itemVO.id = obj["id"].toInt();
    itemVO.itemName = obj["itemName"].toString();
    itemVO.unzipPath = obj["unzipPath"].toString();
    return itemVO;
}

TestDeviceInitInfoVO EftpJsonParser::parseTestDeviceInitInfoVO(const QJsonObject& obj) {
    TestDeviceInitInfoVO initInfoVO;
    initInfoVO.dirWhiteList = obj["dirWhiteList"].toString();

    QJsonArray testItemVOList = obj["testItemVOList"].toArray();
    for (int i = 0; i < testItemVOList.size(); ++i) {
        initInfoVO.testItemVOList.append(parseTestItemVO(testItemVOList[i].toObject()));
    }
    return initInfoVO;
}

TestData EftpJsonParser::parseTestData(const QJsonObject& obj) {
    TestData data;
    data.cycleId = obj["cycleId"].toInt();
    data.initStatus = obj["initStatus"].toInt();
    data.isAutoExecute = obj["isAutoExecute"].toInt();
    data.isInit = obj["isInit"].toInt();
    data.sn = obj["sn"].toString();
    data.status = obj["status"].toInt();
    data.technologyId = obj["technologyId"].toInt();

    QJsonArray stageVOList = obj["testDeviceCycleStageVOList"].toArray();
    for (int i = 0; i < stageVOList.size(); ++i) {
        data.testDeviceCycleStageVOList.append(parseTestDeviceCycleStageVO(stageVOList[i].toObject()));
    }

    if (obj.contains("testDeviceInitInfoVO")) {
        QJsonObject initInfoObj = obj["testDeviceInitInfoVO"].toObject();
        data.testDeviceInitInfoVO = parseTestDeviceInitInfoVO(initInfoObj);
    }
    return data;
}

ComputerTestInfo EftpJsonParser::parseComputerTestInfo(const QString& jsonString) {
    ComputerTestInfo rootObj;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());

    if (!doc.isObject())
        return rootObj;

    QJsonObject obj = doc.object();
    rootObj.code = obj["code"].toInt();
    rootObj.message = obj["message"].toString();
    rootObj.data = parseTestData(obj["data"].toObject());
    return rootObj;
}

// ── 工具方法 ──

void EftpJsonParser::exportLogs(const QString& errorString) {
    QString appDirPath = QCoreApplication::applicationDirPath() + "/Log.text";
    QFile logFile(appDirPath);

    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        out << "[" << currentTime << "] " << errorString << "\n";
        logFile.close();
    } else {
        qWarning("EftpJsonParser: Unable to open log file: %s", qPrintable(logFile.errorString()));
    }
}
