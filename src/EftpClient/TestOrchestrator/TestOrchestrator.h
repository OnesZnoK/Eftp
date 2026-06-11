#pragma once

/**
 * @file TestOrchestrator.h
 * @brief 测试流程状态机（SHARED DLL）
 *
 * 功能：
 *   - 管理测试生命周期（初始化→阶段测试→结果上报）
 *   - 按顺序执行测试项（检查版本→下载→运行→上报）
 *   - 通过信号请求下载，不直接依赖 DownloadManager
 *   - 后台版本检查（定时轮询所有测试项版本）
 *
 * 业务逻辑：
 *   1. 用户点击"开始测试"或自动执行 → start() 启动状态机
 *   2. 遍历每个测试项：
 *      a. 检查本地版本（extractHref/version.txt）与服务端版本是否一致
 *      b. 版本不匹配 → 发射 requestDownload 信号，等待下载完成
 *      c. 版本匹配 → 直接运行外部测试程序
 *      d. QProcess 启动程序，传参：cycleId, updateUrl, testId, isRetry, queryUrl
 *      e. 程序退出后上报 cycleItemEnd，查询 queryCycleTestItemInfo 获取结果
 *      f. 发射 testItemCompleted 通知 UI 更新
 *   3. 当前阶段所有项完成 → 进入下一阶段
 *   4. 所有阶段完成 → 发射 allTestsCompleted
 *
 * 后台版本检查：
 *   - 测试开始时启动定时器（默认 5 分钟）
 *   - 轮询所有测试项版本，不匹配时触发后台下载
 *   - 跳过当前正在测试的项（避免冲突）
 *
 * 信号流：
 *   TestOrchestrator ──requestDownload──→ MainWindow ──→ DownloadManager
 *   DownloadManager ──downloadCompleted──→ MainWindow ──→ TestOrchestrator::onDownloadCompleted
 */

#include <QObject>
#include <QString>
#include <QVariant>
#include <QTimer>
#include "EftpTypes.h"

#include <QtCore/qglobal.h>

#ifdef TESTORCHESTRATOR_LIBRARY
#  define TESTORCHESTRATOR_EXPORT Q_DECL_EXPORT
#else
#  define TESTORCHESTRATOR_EXPORT Q_DECL_IMPORT
#endif

/**
 * @brief 测试流程状态机
 *
 * 状态流转：
 *   Idle → CheckVersion → Download → RunProgram → ReportResult → NextItem → (循环)
 *                                                      ↓
 *                                               StageComplete → AllComplete
 */
class TESTORCHESTRATOR_EXPORT TestOrchestrator : public QObject
{
    Q_OBJECT

public:
    explicit TestOrchestrator(QObject* parent = nullptr);
    ~TestOrchestrator();

    /**
     * @brief 设置测试计划数据
     * @param plan 从 SerApiModel 获取的测试计划
     */
    void setTestPlan(const TestPlanInfo& plan);

    /**
     * @brief 设置设备信息（用于上报）
     * @param sn 设备序列号
     */
    void setDeviceSn(const std::string& sn);

    /**
     * @brief 启动测试流程
     * @param stageIndex 从哪个阶段开始（默认 0）
     */
    void start(int stageIndex = 0);

    /**
     * @brief 重测指定测试项
     * @param cycleItemId 测试项ID
     */
    void retest(int cycleItemId);

    /**
     * @brief 启动后台版本检查定时器
     * @param intervalMs 检查间隔（毫秒），默认 5 分钟
     */
    void startVersionCheck(int intervalMs = 300000);

    /**
     * @brief 停止后台版本检查
     */
    void stopVersionCheck();

    /**
     * @brief 标记当前正在测试的项（后台检查时跳过）
     */
    void setTestingItem(int stageIndex, int itemIndex);

    /**
     * @brief 清除当前测试项标记
     */
    void clearTestingItem();

public slots:
    /**
     * @brief 接收下载完成通知（由 MainWindow 路由自 DownloadManager）
     * @param requestId 请求标识
     * @param localPath 本地路径
     * @param success 是否成功
     */
    void onDownloadCompleted(int requestId, const QString& localPath, bool success);

signals:
    /** @brief 请求下载（MainWindow 路由到 DownloadManager） */
    void requestDownload(int requestId, const QString& url, const QString& localPath);

    /** @brief 测试流程通知（MainWindow 更新 UI） */
    void testStarted();
    void testStageStarted(int stageIndex, const QString& stageName);
    void testItemStarted(int stageIndex, int itemIndex, const QString& itemName);
    void testItemCompleted(int stageIndex, int itemIndex, int result);
    void testStageCompleted(int stageIndex, bool success);
    void allTestsCompleted(bool success);
    void testError(const QString& errorMsg);
    void showTips(const QString& message);  ///< 显示提示信息（MainWindow 更新 UI）

    /** @brief 内部信号：版本检查结果（跨线程回调） */
    void versionCheckResult(bool needUpdate, const QString& callHref,
                           const QString& extractPath, const QString& downloadUrl,
                           const QString& itemName);
    /** @brief 内部信号：步骤就绪（跨线程回调） */
    void stepReady(int nextStep);

public slots:
    /** @brief 接收版本检查结果（主线程） */
    void onVersionCheckResult(bool needUpdate, const QString& callHref,
                             const QString& extractPath, const QString& downloadUrl,
                             const QString& itemName);
    /** @brief 接收步骤就绪通知（主线程） */
    void onStepReady(int nextStep);

private:
    /** @brief 状态机步骤 */
    enum class Step {
        Idle,           // 空闲
        CheckVersion,   // 检查版本
        Download,       // 等待下载
        RunProgram,     // 运行测试程序
        ReportResult,   // 上报结果
        NextItem        // 下一个测试项
    };

    /**
     * @brief 执行当前测试项的下一步
     *        根据 m_currentStep 分发到 doCheckVersion/doRunProgram/doReportResult/doNextItem
     */
    void executeCurrentStep();

    /**
     * @brief 检查版本
     *        调用 SerApiModel::checkTestItemVersion，判断是否需要下载
     */
    void doCheckVersion();

    /**
     * @brief 运行测试程序
     *        启动 QProcess 执行外部测试程序，等待完成后上报
     */
    void doRunProgram();

    /**
     * @brief 上报测试结果
     *        调用 SerApiModel::cycleItemEnd + queryCycleTestItemInfo
     */
    void doReportResult();

    /**
     * @brief 移动到下一个测试项
     *        如果当前阶段所有项完成，进入下一个阶段
     */
    void doNextItem();

    // ── 数据 ──
    TestPlanInfo m_plan;
    std::string m_sn;
    int m_requestIdCounter = 0;

    // ── 当前执行位置 ──
    int m_currentStage = 0;
    int m_currentItem = 0;
    Step m_currentStep = Step::Idle;
    int m_currentRequestId = 0;

    // ── 当前测试项的程序路径（从 checkTestItemVersion 获取）──
    std::string m_currentCallHref;
    std::string m_currentExtractPath;

    // ── 后台版本检查 ──
    QTimer m_versionCheckTimer;
    int m_testingStage = -1;   // 当前正在测试的阶段（-1=无）
    int m_testingItem = -1;    // 当前正在测试的项（-1=无）

private slots:
    void checkBackgroundVersions();
};
