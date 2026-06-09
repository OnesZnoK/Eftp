#include "TestOrchestrator.h"
#include "SerApiModel.h"
#include "ConfigManager.h"
#include "qtlogger.h"

#include <QtConcurrent>
#include <QProcess>
#include <QFile>
#include <QDir>

TestOrchestrator::TestOrchestrator(QObject* parent)
    : QObject(parent)
{
    // 内部信号→槽连接（跨线程：worker线程→主线程）
    connect(this, &TestOrchestrator::versionCheckResult,
            this, &TestOrchestrator::onVersionCheckResult, Qt::QueuedConnection);
    connect(this, &TestOrchestrator::stepReady,
            this, &TestOrchestrator::onStepReady, Qt::QueuedConnection);

    // 后台版本检查定时器
    connect(&m_versionCheckTimer, &QTimer::timeout,
            this, &TestOrchestrator::checkBackgroundVersions);
}

TestOrchestrator::~TestOrchestrator()
{
}

/**
 * @brief 公开接口
 */

void TestOrchestrator::setTestPlan(const TestPlanInfo& plan)
{
    m_plan = plan;
}

void TestOrchestrator::setDeviceSn(const std::string& sn)
{
    m_sn = sn;
}

void TestOrchestrator::start(int stageIndex)
{
    if (m_plan.stages.empty()) {
        emit testError("测试计划为空");
        return;
    }

    m_currentStage = stageIndex;
    m_currentItem = 0;
    m_currentStep = Step::Idle;

    // 启动后台版本检查（测试开始时启动，每 5 分钟轮询一次）
    startVersionCheck(300000);

    emit testStarted();
    QtLogger::WriteLog(QString("TestOrchestrator: 启动测试，阶段 %1/%2")
        .arg(m_currentStage + 1).arg(m_plan.stages.size()));

    // 通知当前阶段开始
    emit testStageStarted(m_currentStage,
        QString::fromStdString(m_plan.stages[m_currentStage].stageName));

    // 直接检查第一个测试项版本（不经过 doNextItem，避免跳过 item 0）
    m_currentStep = Step::CheckVersion;
    executeCurrentStep();
}

/**
 * @brief 槽函数：接收下载完成通知（由 MainWindow 路由自 DownloadManager）
 */

void TestOrchestrator::onDownloadCompleted(int requestId, const QString& localPath, bool success)
{
    if (requestId != m_currentRequestId) {
        return;
    }

    QtLogger::WriteLog(QString("TestOrchestrator: 下载完成 #%1, 成功=%2")
        .arg(requestId).arg(success));

    if (!success) {
        QtLogger::WriteLog("TestOrchestrator: 下载失败，跳过该测试项", enLogType::WARNING);
        m_currentStep = Step::NextItem;
        executeCurrentStep();
        return;
    }

    m_currentStep = Step::RunProgram;
    executeCurrentStep();
}

/**
 * @brief 内部状态机（根据 m_currentStep 执行对应逻辑）
 */

void TestOrchestrator::executeCurrentStep()
{
    switch (m_currentStep) {
    case Step::CheckVersion:
        doCheckVersion();
        break;
    case Step::Download:
        // 下载由信号触发，这里等待 onDownloadCompleted 回调
        break;
    case Step::RunProgram:
        doRunProgram();
        break;
    case Step::ReportResult:
        doReportResult();
        break;
    case Step::NextItem:
        doNextItem();
        break;
    default:
        break;
    }
}

/**
 * @brief 检查当前测试项的版本
 *        读取本地 version.txt，调用 API 比对版本
 *        版本不匹配时发射 requestDownload 信号
 */

void TestOrchestrator::doCheckVersion()
{
    const auto& stage = m_plan.stages[m_currentStage];
    if (m_currentItem >= (int)stage.items.size()) {
        m_currentStep = Step::NextItem;
        executeCurrentStep();
        return;
    }

    const auto& item = stage.items[m_currentItem];
    int cycleItemId = item.cycleItemId;

    QtLogger::WriteLog(QString("TestOrchestrator: 检查版本 %1 (cycleItemId=%2)")
        .arg(QString::fromStdString(item.itemName)).arg(cycleItemId));

    // 通知 UI：测试项开始运行（变黄）
    emit testItemStarted(m_currentStage, m_currentItem,
        QString::fromStdString(item.itemName));

    // 后台线程执行 API 调用
    QtConcurrent::run([=]() {
        // 从测试项数据中获取 extractHref，读取本地版本
        std::string localVersion = "";
        std::string extractPath = item.extractHref;
        if (!extractPath.empty()) {
            QString versionFile = QString::fromStdString(extractPath) + "/version.txt";
            QFile f(versionFile);
            if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                localVersion = QString::fromUtf8(f.readAll().trimmed()).toStdString();
                f.close();
                QtLogger::WriteLog("TestOrchestrator: 本地版本 " + QString::fromStdString(localVersion));
            } else {
                QtLogger::WriteLog("TestOrchestrator: 版本文件不存在 " + versionFile);
            }
        } else {
            QtLogger::WriteLog("TestOrchestrator: extractHref 为空");
        }

        SerApiModel api;
        QtLogger::WriteLog("TestOrchestrator: 调用 checkTestItemVersion...");
        VersionInfo ver = api.checkTestItemVersion(cycleItemId, localVersion);
        QtLogger::WriteLog(QString("TestOrchestrator: checkTestItemVersion 返回, needUpdate=%1")
            .arg(ver.needUpdate));

        // 通过信号回到主线程（用 QString 避免 std::string 跨线程序列化问题）
        QtLogger::WriteLog("TestOrchestrator: 发射 versionCheckResult 信号");
        emit versionCheckResult(ver.needUpdate,
            QString::fromStdString(ver.callHref),
            QString::fromStdString(ver.extractPath),
            QString::fromStdString(ver.downloadUrl),
            QString::fromStdString(ver.itemName));
        QtLogger::WriteLog("TestOrchestrator: 信号已发射");
    });
}

/**
 * @brief 版本检查结果回调（主线程，从后台线程通过信号触发）
 *        版本匹配则直接运行程序，不匹配则请求下载
 */

void TestOrchestrator::onVersionCheckResult(bool needUpdate, const QString& callHref,
                                            const QString& extractPath, const QString& downloadUrl,
                                            const QString& itemName)
{
    QtLogger::WriteLog(QString("TestOrchestrator: onVersionCheckResult, needUpdate=%1")
        .arg(needUpdate));

    m_currentCallHref = callHref.toStdString();
    m_currentExtractPath = extractPath.toStdString();

    if (needUpdate) {
        // 需要下载
        QtLogger::WriteLog("TestOrchestrator: 版本需要更新，请求下载 " + itemName);
        m_currentRequestId = ++m_requestIdCounter;
        m_currentStep = Step::Download;
        emit requestDownload(m_currentRequestId, downloadUrl, extractPath);
    } else {
        // 版本正确，直接运行程序
        QtLogger::WriteLog("TestOrchestrator: 版本正确，直接运行");
        m_currentStep = Step::RunProgram;
        executeCurrentStep();
    }
}

/**
 * @brief 步骤就绪回调（主线程，从 worker 线程通过信号触发）
 */

void TestOrchestrator::onStepReady(int nextStep)
{
    m_currentStep = static_cast<Step>(nextStep);
    QtLogger::WriteLog(QString("TestOrchestrator: onStepReady, step=%1").arg(nextStep));
    executeCurrentStep();
}

/**
 * @brief 运行测试程序（主线程启动 QProcess，信号通知完成）
 */

void TestOrchestrator::doRunProgram()
{
    const auto& stage = m_plan.stages[m_currentStage];
    const auto& item = stage.items[m_currentItem];

    QtLogger::WriteLog(QString("TestOrchestrator: 运行测试 %1, 程序路径=%2")
        .arg(QString::fromStdString(item.itemName))
        .arg(QString::fromStdString(item.callHref)));

    emit testItemStarted(m_currentStage, m_currentItem,
        QString::fromStdString(item.itemName));

    // 后台线程运行：阻塞等待外部程序结束
    QtConcurrent::run([=]() {
        int cycleItemId = item.cycleItemId;

        // 1. 上报测试项开始
        SerApiModel api;
        api.cycleItemStart(cycleItemId);
        QtLogger::WriteLog(QString("TestOrchestrator: 上报测试开始 cycleItemId=%1").arg(cycleItemId));

        // 2. 构造参数：cycleId, updateUrl, testId, isRetry, queryUrl
        QString updateUrl = QString::fromStdString(ConfigManager::instance().buildUrl("reportCycleTestData"));
        QString queryUrl = QString::fromStdString(ConfigManager::instance().buildUrl("queryTestItemParam"));
        QStringList args = {
            QString::number(cycleItemId),
            updateUrl,
            QString::number(item.itemId),
            "0",
            queryUrl
        };

        // 3. 启动外部程序（阻塞等待）
        QString programPath = QString::fromStdString(item.callHref);
        QProcess process;
        process.setWorkingDirectory(QDir::currentPath());
        process.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
        process.start(programPath, args);

        QtLogger::WriteLog(QString("TestOrchestrator: 启动程序 %1 %2")
            .arg(programPath).arg(args.join(" ")));

        // 阻塞等待外部程序结束
        bool finished = process.waitForFinished(-1);

        if (finished) {
            int exitCode = process.exitCode();
            QString stderr_output = process.readAllStandardError();
            QtLogger::WriteLog(QString("TestOrchestrator: 程序退出 exitCode=%1").arg(exitCode));
            if (!stderr_output.isEmpty()) {
                QtLogger::WriteLog("TestOrchestrator: stderr=" + stderr_output);
            }
        } else {
            QtLogger::WriteLog("TestOrchestrator: 程序启动失败或超时", enLogType::WARNING);
            process.kill();
        }

        // 4. 上报结果（不管程序是否成功都上报）
        SerApiModel api2;
        api2.cycleItemEnd(cycleItemId);

        // 等待服务器处理完程序上报的结果，最多重试 3 次（每次等 3 秒）
        TestItemResult result;
        for (int retry = 0; retry < 3; retry++) {
            result = api2.queryCycleTestItemInfo(cycleItemId);
            if (result.testResult != 0) break;
            QtLogger::WriteLog(QString("TestOrchestrator: result=0, 等待 3 秒重试 (%1/3)").arg(retry + 1));
            QThread::sleep(3);
        }

        QtLogger::WriteLog(QString("TestOrchestrator: 测试结果 %1, result=%2")
            .arg(QString::fromStdString(item.itemName))
            .arg(result.testResult));

        // 5. 通知 UI + 下一个测试项
        emit testItemCompleted(m_currentStage, m_currentItem, result.testResult);
        emit stepReady(static_cast<int>(Step::NextItem));
    });
}

/**
 * @brief 上报结果（已合并到 doRunProgram 的 QProcess::finished 回调中）
 */

void TestOrchestrator::doReportResult()
{
    // 此方法不再需要，结果上报已在 doRunProgram 的 finished 回调中完成
    // 保留为空实现以兼容状态机
}

/**
 * @brief 移动到下一个测试项
 *        当前阶段所有项完成时，进入下一个阶段
 */

void TestOrchestrator::doNextItem()
{
    const auto& stage = m_plan.stages[m_currentStage];

    m_currentItem++;

    // 当前阶段还有测试项
    if (m_currentItem < (int)stage.items.size()) {
        m_currentStep = Step::CheckVersion;
        executeCurrentStep();
        return;
    }

    // 当前阶段完成
    QtLogger::WriteLog(QString("TestOrchestrator: 阶段 %1 完成")
        .arg(QString::fromStdString(stage.stageName)));
    emit testStageCompleted(m_currentStage, true);

    // 进入下一个阶段
    m_currentStage++;
    if (m_currentStage < (int)m_plan.stages.size()) {
        m_currentItem = 0;

        emit testStageStarted(m_currentStage,
            QString::fromStdString(m_plan.stages[m_currentStage].stageName));

        m_currentStep = Step::CheckVersion;
        executeCurrentStep();
        return;
    }

    // 所有阶段完成
    QtLogger::WriteLog("TestOrchestrator: 所有测试完成");
    emit allTestsCompleted(true);
    m_currentStep = Step::Idle;
}

/**
 * @brief 后台版本检查（合并自 VersionChecker）
 *        定时轮询所有测试项的版本，不匹配时触发下载
 */

void TestOrchestrator::startVersionCheck(int intervalMs)
{
    QtLogger::WriteLog(QString("TestOrchestrator: 启动后台版本检查，间隔 %1 秒").arg(intervalMs / 1000));
    m_versionCheckTimer.start(intervalMs);
}

void TestOrchestrator::stopVersionCheck()
{
    m_versionCheckTimer.stop();
    QtLogger::WriteLog("TestOrchestrator: 停止后台版本检查");
}

void TestOrchestrator::setTestingItem(int stageIndex, int itemIndex)
{
    m_testingStage = stageIndex;
    m_testingItem = itemIndex;
}

void TestOrchestrator::clearTestingItem()
{
    m_testingStage = -1;
    m_testingItem = -1;
}

void TestOrchestrator::checkBackgroundVersions()
{
    QtLogger::WriteLog("TestOrchestrator: 后台版本检查开始");

    int checkedCount = 0;
    int outdatedCount = 0;

    for (int s = 0; s < (int)m_plan.stages.size(); s++) {
        const auto& stage = m_plan.stages[s];

        for (int i = 0; i < (int)stage.items.size(); i++) {
            // 跳过当前正在测试的项
            if (s == m_testingStage && i == m_testingItem) {
                QtLogger::WriteLog(QString("TestOrchestrator: 跳过正在测试的项 %1")
                    .arg(QString::fromStdString(stage.items[i].itemName)));
                continue;
            }

            const auto& item = stage.items[i];

            // 读取本地版本
            std::string localVersion;
            if (!item.extractHref.empty()) {
                QString versionFile = QString::fromStdString(item.extractHref) + "/version.txt";
                QFile f(versionFile);
                if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    localVersion = QString::fromUtf8(f.readAll().trimmed()).toStdString();
                    f.close();
                }
            }

            // 检查版本（在后台线程执行）
            SerApiModel api;
            VersionInfo ver = api.checkTestItemVersion(item.cycleItemId, localVersion);
            checkedCount++;

            if (ver.needUpdate) {
                outdatedCount++;
                QtLogger::WriteLog(QString("TestOrchestrator: %1 版本过旧，请求下载")
                    .arg(QString::fromStdString(item.itemName)));

                int requestId = ++m_requestIdCounter;
                emit requestDownload(requestId,
                    QString::fromStdString(ver.downloadUrl),
                    QString::fromStdString(ver.extractPath));
            }
        }
    }

    QtLogger::WriteLog(QString("TestOrchestrator: 后台版本检查完成，共 %1 项，需更新 %2 项")
        .arg(checkedCount).arg(outdatedCount));
}
