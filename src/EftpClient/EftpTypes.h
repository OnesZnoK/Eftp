#pragma once
/**
 * @file EftpTypes.h
 * @brief EFTP 项目统一数据结构定义
 *
 * 本文件定义了项目中所有模块共享的数据结构，分为两大类：
 *   1. API 数据结构（std::string） — 用于 SerApiModel 与服务端通信
 *   2. UI 数据结构（QString）      — 用于 Widget 层显示和交互
 *
 * 设计原则：
 *   - API 层使用 std::string（与 cpp-httplib 一致）
 *   - UI 层使用 QString（与 Qt Widgets 一致）
 *   - 两套结构可以共存，MainWindow 负责转换
 */

#include <string>
#include <vector>

#include "json.hpp"
using json = nlohmann::json;

#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMetaType>

/**
 * @brief 枚举定义
 */

/** 主页面 TabWidget 标签页 */
enum TabIndex { Initial = 0, Update, Setting, AutoTesting, ManualTesting, Clear };

/** 初始化页面列表列 */
enum EDYDelegateColumn { TestDesc = 0, State, BtnText, ErrorText };

/** 是否需要初始化 */
enum InitialFlag { NoInit = 0, NeedInit };

/** 初始化状态 */
enum InitialStatu { None = 0, InitRuning, InitDone };

/** 测试项状态 */
enum TestState { UnStart = 0, Running, Success, Fail };

/** 测试项结果 */
enum TestResult { ResultUnknown = 0, ResultSuccess, ResultFail };

/** 阶段状态 */
enum PhaseState { PhaseStart = 0, PhaseRunning, PhaseEnd };

/** 测试项结果详细信息列 */
enum TreeWidgetColumnIndex { Name = 0, Result, FailReason, Detail };

/** 测试程序详情索引 */
enum TestProgramDetailIndex { ProgramName = Qt::DisplayRole, ProgramState, ResultInfo };

/** 拖曳大小宽度 */
constexpr int boundaryWidth = 4;

/** 树视图列数 */
constexpr int treeviewColumn = 8;

/**
 * @brief API 数据结构（std::string，用于服务端通信）
 */

/**
 * @brief 通用 API 响应
 */
struct ApiResponse {
    bool success = false;           // 请求是否成功（code==0 且 HTTP 2xx）
    int code = -1;                  // 服务端业务状态码（0=成功, 1001=未绑定, 1003=绑定不匹配）
    std::string message;            // 服务端返回的消息
    json data;                      // 服务端返回的 data 字段
    std::string rawBody;            // 完整的 HTTP 响应体原文
};

/**
 * @brief 设备基础信息（来自 queryDeviceInfo）
 */
struct DeviceBaseDataInfo {
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
    std::string errorMessage;       // API 失败时的错误信息
};

/**
 * @brief 工序路由信息（来自 queryDeviceRouteInfo）
 */
struct DeviceRouteDataInfo {
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
struct TestItemResult {
    int cycleItemId = 0;            // 运行时测试项ID（deviceCycleItemId）
    int itemId = 0;                 // 测试项静态ID（用于上报结果）
    std::string itemName;           // 测试项名称
    int testResult = 0;             // 测试结果：0=无, 1=通过, 2=失败
    std::string failReason;         // 失败原因
    std::string detail;             // 详细信息
    std::string callHref;           // 程序路径（exe）
    std::string extractHref;        // 安装目录（含 version.txt）
    int isRepeatTest = 0;           // 是否支持重测：0=否, 1=是
};

/**
 * @brief 测试阶段
 */
struct TestStageInfo {
    int stageId = 0;                // 阶段ID
    std::string stageName;          // 阶段名称
    int stageState = 0;             // 阶段状态：0=未开始, 1=运行中, 2=完成
    int isAutoExecute = 0;          // 是否自动执行：0=手动, 1=自动
    std::vector<TestItemResult> items;  // 该阶段下的所有测试项
};

/**
 * @brief 测试项VO（初始化阶段使用）
 */
struct TestItemVO {
    int id = 0;                     // 主键ID
    QString itemName = "";          // 测试项名称
    QString checkVersionResult = ""; // 版本是否匹配：0=不匹配, 1=匹配
    QString downLoadPath = "";      // 程序包下载路径
    QString unzipPath = "";         // 解压路径
};

/**
 * @brief 初始化信息
 */
struct TestDeviceInitInfoVO {
    QString dirWhiteList = "";            // 磁盘白名单（Defender 排除路径）
    QList<TestItemVO> testItemVOList;     // 初始化测试项列表
};

/**
 * @brief 完整测试计划（来自 queryDeviceTestInfo）
 */
struct TestPlanInfo {
    int cycleId = 0;                // 测试周期ID
    int technologyId = 0;           // 测试工艺ID
    int isInit = 0;                 // 是否需要初始化：0=不需要, 1=需要
    int initStatus = 0;             // 初始化状态：0=未开始, 1=进行中, 2=已完成
    int isAutoExecute = 0;          // 是否自动执行：0=否, 1=是
    int status = 0;                 // 状态：0=未开始, 1=测试中, 2=已完成, 3=已关闭
    std::string sn;                 // 设备序列号
    std::vector<TestStageInfo> stages;  // 所有测试阶段列表
    TestDeviceInitInfoVO initInfo;  // 初始化信息（isInit=1 时有数据）
    std::string errorMessage;       // API 失败时的错误信息
};

/**
 * @brief 测试项版本信息（来自 checkTestItemVersion）
 */
struct VersionInfo {
    bool needUpdate = false;        // 是否需要更新（serverVersion != localVersion）
    std::string version;            // 服务端版本号
    std::string downloadUrl;        // 程序包下载路径（downLoadPath）
    std::string extractPath;        // 解压路径（extractHref）
    std::string callHref;           // 程序入口路径
    std::string itemName;           // 测试项名称
    std::string programName;        // 程序名称
    int programType = 0;            // 程序类型：1=exe, 2=bat
    bool isAutoExecute = false;     // 是否自动执行
    bool isRepeatTest = false;      // 是否支持重测
    std::string tips;               // 知识库提示
    std::string md5;                // 【预留】文件 MD5 校验值
};

/**
 * @brief UI 数据结构（QString，用于 Widget 层显示）
 */

/**
 * @brief 基础响应信息
 */
struct BaseInfo {
    int code = 0;
    QString message = "";
};

/**
 * @brief 初始化列表测试项信息
 */
struct EDYTreeModelInfo {
    QString testItemDsc = "";       // 测试项描述
    TestState state = TestState::UnStart; // 状态
    QString btnText = "";           // 按钮文字
    bool isBtnEnable = true;        // 按钮是否可用
    QString errorText = "";         // 错误信息
};

/**
 * @brief 测试程序详情信息
 */
struct ItemProgramDetailInfo {
    QString programName = "";       // 程序名称
    TestState programState = TestState::UnStart; // 程序状态
    QString resultInfo = "";        // 结果信息（JSON）
};

/**
 * @brief 上传非法关机、蓝屏文件信息
 */
struct UploadDumpFileInfo {
    QString sn = "";                // 设备序列号
    QString mac = "";               // MAC 地址
    QString dataTime = "";          // 数据时间（dump 生成时间）
    QString uploadTime = "";        // 上传时间
    QString fileUrl = "";           // 服务器返回的文件 URL
    QString type = "";              // 类型：0=意外关机, 1=蓝屏文件

    QString toJsonString() const {
        QJsonObject obj;
        obj["sn"] = sn;
        obj["mac"] = mac;
        obj["dataTime"] = dataTime;
        obj["uploadTime"] = uploadTime;
        obj["fileUrl"] = fileUrl;
        obj["type"] = type;
        QJsonDocument doc(obj);
        return QString(doc.toJson(QJsonDocument::Compact));
    }
};

/**
 * @brief 测试项数据
 */
struct TestItemData {
    QString key = "";               // 数据 key
    QString name = "";              // 数据名称
    int valueType = 0;              // 值类型
    QString valueTypeDesc = "";     // 值类型描述
};

/**
 * @brief 测试项参数
 */
struct TestItemParam {
    QString key = "";               // 参数 key
    QString name = "";              // 参数名称
    QString value = "";             // 参数值
};

/**
 * @brief 关联数据
 */
struct RelationDate {
    QString key = "";               // 关联 key
    QString name = "";              // 关联名称
    int valueType = 0;              // 值类型
    QString valueTypeDesc = "";     // 值类型描述
};

/**
 * @brief 测试项规则
 */
struct TestItemRule {
    QString errorInfo = "";         // 错误信息
    bool isContinue = 0;            // 是否继续
    QList<RelationDate> relationDateList; // 关联数据列表
    QString ruleCode = "";          // 规则代码
    QString ruleName = "";          // 规则名称
    int status = 0;                 // 状态
    QString statusDesc = "";        // 状态描述
    QString testStandard = "";      // 测试标准
};

/**
 * @brief 测试项SPU关联
 */
struct TestItemSpuRelation {
    int spuId = 0;                  // SPU ID
    QString spuName = "";           // SPU 名称
};

/**
 * @brief 版本数据信息（详细版，用于UI显示）
 */
struct VersionDataInfo {
    int applicableModelsType = 0;   // 适用机型类型
    QString callHref = "";          // 程序入口路径
    QString checkVersionResult = ""; // 版本检查结果：0=不匹配, 1=匹配
    QString createBy = "";          // 创建人
    QString createTime = "";        // 创建时间
    QString downLoadPath = "";      // 下载路径
    QString extractHref = "";       // 解压路径
    QString id = "";                // 主键ID
    bool isOffline = 0;             // 是否离线
    bool isRepeatTest = 0;          // 是否支持重测
    QString itemName = "";          // 测试项名称
    int productType = 0;            // 产品类型
    QString productTypeDesc = "";   // 产品类型描述
    int programType = 0;            // 程序类型
    QString programTypeDesc = "";   // 程序类型描述
    int status = 0;                 // 状态
    QString statusDesc = "";        // 状态描述
    QList<TestItemData> testItemDataList;       // 测试项数据列表
    QList<TestItemParam> testItemParamList;     // 测试项参数列表
    QList<TestItemRule> testItemRuleList;       // 测试项规则列表
    QList<TestItemSpuRelation> testItemSpuRelationList; // SPU关联列表
    QString tips = "";              // 知识库提示
    QString updateBy = "";          // 更新人
    QString updateTime = "";        // 更新时间
    QString version = "";           // 版本号
    QString unzipPath = "";         // 解压路径

    QString toJsonString() const {
        QJsonObject obj;
        obj["applicableModelsType"] = applicableModelsType;
        obj["callHref"] = callHref;
        obj["checkVersionResult"] = checkVersionResult;
        obj["createBy"] = createBy;
        obj["createTime"] = createTime;
        obj["downLoadPath"] = downLoadPath;
        obj["extractHref"] = extractHref;
        obj["id"] = id;
        obj["isOffline"] = isOffline;
        obj["isRepeatTest"] = isRepeatTest;
        obj["itemName"] = itemName;
        obj["productType"] = productType;
        obj["programType"] = programType;
        obj["productTypeDesc"] = productTypeDesc;
        obj["programTypeDesc"] = programTypeDesc;
        obj["status"] = status;
        obj["statusDesc"] = statusDesc;
        obj["tips"] = tips;
        obj["updateBy"] = updateBy;
        obj["updateTime"] = updateTime;
        obj["version"] = version;
        obj["unzipPath"] = unzipPath;
        QJsonDocument doc(obj);
        return QString(doc.toJson(QJsonDocument::Compact));
    }
};
Q_DECLARE_METATYPE(VersionDataInfo)

/**
 * @brief 测试设备周期项数据VO
 */
struct TestDeviceCycleItemDataVO {
    QString name = "";              // 数据名称
    QString value = "";             // 数据值

    QJsonObject toJsonObject() const {
        QJsonObject obj;
        obj["name"] = name;
        obj["value"] = value;
        return obj;
    }
};

/**
 * @brief 测试设备周期项结果VO
 */
struct TestDeviceCycleItemResultVO {
    QString errorInfo = "";         // 错误信息
    int result = 0;                 // 结果：0=失败, 1=通过
    QString ruleName = "";          // 规则名称
    QList<TestDeviceCycleItemDataVO> testDeviceCycleItemDataVOList; // 关联数据列表
    QString testStandard = "";      // 测试标准

    QJsonObject toJsonObject() const {
        QJsonObject obj;
        obj["errorInfo"] = errorInfo;
        obj["result"] = result;
        obj["ruleName"] = ruleName;
        obj["testStandard"] = testStandard;
        QJsonArray dataArray;
        for (const auto& dataVO : testDeviceCycleItemDataVOList) {
            dataArray.append(dataVO.toJsonObject());
        }
        obj["testDeviceCycleItemDataVOList"] = dataArray;
        return obj;
    }
};

/**
 * @brief 测试设备周期项VO
 */
struct TestDeviceCycleItemVO {
    QString callHref = "";          // 程序入口路径（exe）
    int deviceCycleId = 0;          // 设备生命周期ID
    int deviceCycleItemId = 0;      // 生命周期测试项ID
    int deviceCycleStageId = 0;     // 生命周期阶段ID
    QString extractHref = "";       // 解压路径（含 version.txt）
    int id = 0;                     // 主键ID
    int isOffline = 0;              // 是否离线：0=在线, 1=离线(U盘)
    int isRepeatTest = 0;           // 是否支持重测：0=否, 1=是
    int itemId = 0;                 // 测试项静态ID
    QString itemName = "";          // 测试项名称
    int productType = 0;            // 产品类型
    int programType = 0;            // 程序类型：1=exe, 2=bat
    int result = 0;                 // 结果：0=无, 1=通过, 2=失败
    int stageId = 0;                // 阶段ID
    int status = 0;                 // 状态：0=未开始, 1=进行中, 2=已完成
    int technologyId = 0;           // 工艺ID
    QList<TestDeviceCycleItemResultVO> testDeviceCycleItemResultVOList; // 规则结果列表
    QString tips = "";              // 知识库提示

    QString toJsonString() const {
        QJsonObject obj;
        obj["callHref"] = callHref;
        obj["deviceCycleId"] = deviceCycleId;
        obj["deviceCycleItemId"] = deviceCycleItemId;
        obj["deviceCycleStageId"] = deviceCycleStageId;
        obj["extractHref"] = extractHref;
        obj["id"] = id;
        obj["isOffline"] = isOffline;
        obj["isRepeatTest"] = isRepeatTest;
        obj["itemId"] = itemId;
        obj["itemName"] = itemName;
        obj["productType"] = productType;
        obj["programType"] = programType;
        obj["result"] = result;
        obj["stageId"] = stageId;
        obj["status"] = status;
        obj["technologyId"] = technologyId;
        obj["tips"] = tips;
        QJsonArray resultsArray;
        for (const auto& resultVO : testDeviceCycleItemResultVOList) {
            resultsArray.append(resultVO.toJsonObject());
        }
        obj["testDeviceCycleItemResultVOList"] = resultsArray;
        QJsonDocument doc(obj);
        return QString(doc.toJson(QJsonDocument::Compact));
    }
};
Q_DECLARE_METATYPE(TestDeviceCycleItemVO)

/**
 * @brief 测试设备周期阶段VO
 */
struct TestDeviceCycleStageVO {
    int deviceCycleId = 0;          // 设备生命周期ID
    int deviceCycleStageId = 0;     // 生命周期阶段ID
    bool isAutoExecute = 0;         // 是否自动执行
    QString endTime = "";           // 结束时间
    int id = 0;                     // 主键ID
    QString result = "";            // 结果
    QString sn = "";                // 设备序列号
    int stageId = 0;                // 阶段ID
    QString stageName = "";         // 阶段名称
    QString startTime = "";         // 开始时间
    int status = 0;                 // 状态：0=未开始, 1=进行中, 2=已完成
    QList<QList<TestDeviceCycleItemVO>> testDeviceCycleItemVOList; // 测试项列表（二维数组）
};



/**
 * @brief 测试数据（完整测试计划，UI版）
 */
struct TestData {
    int cycleId = 0;                     // 测试周期ID
    int initStatus = 0;                  // 初始化状态：0=未开始, 1=进行中, 2=已完成
    int isAutoExecute = 0;               // 是否自动执行：0=否, 1=是
    int isInit = 0;                      // 是否需要初始化：0=不需要, 1=需要
    QString sn = "";                     // 设备序列号
    int status = 0;                      // 状态：0=未开始, 1=测试中, 2=已完成, 3=已关闭
    int technologyId = 0;                // 测试工艺ID
    QList<TestDeviceCycleStageVO> testDeviceCycleStageVOList; // 执行阶段列表
    TestDeviceInitInfoVO testDeviceInitInfoVO;                // 初始化信息
};

/**
 * @brief 测试信息（UI版完整响应）
 */
struct ComputerTestInfo : public BaseInfo {
    TestData data;                       // 测试数据
    QString message = "";                // 消息
    int code = -1;                       // 状态码
};

/**
 * @brief 设备硬件信息（采集结果）
 */
struct DeviceInfo {
    QString deviceSn;       // 设备序列号（BIOS SN）
    QString deviceMac;      // 有线网卡 MAC 地址
    QString baseboardSn;    // 主板序列号
};

// 注册自定义类型（跨线程信号/槽需要）
Q_DECLARE_METATYPE(DeviceInfo)
Q_DECLARE_METATYPE(DeviceBaseDataInfo)
Q_DECLARE_METATYPE(TestPlanInfo)
