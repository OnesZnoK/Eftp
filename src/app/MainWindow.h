#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"
#include "EftpTypes.h"

class DeviceModel;
class SerApiModel;
class DownloadManager;
class TestOrchestrator;
class NetworkMonitor;

class InitialPage;
class StageTestPage;
class SystemTrayManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QString& env, const QString& sn, QWidget* parent = nullptr);
    ~MainWindow();

private:
    void createModules();       ///< 创建所有业务模块实例
    void connectSignals();      ///< 连接模块间信号/槽
    void setupUI();             ///< 初始化 UI 布局
    void startWorkflow();       ///< 启动工作流（设备采集）
    void createTabPages(const TestPlanInfo& plan);  ///< 根据测试计划创建 Tab 页面

    QString m_env;              ///< 运行环境（product/pre/test）
    QString m_sn;               ///< 设备序列号

    DeviceModel*       m_deviceModel  = nullptr;  ///< 设备硬件信息采集
    SerApiModel*       m_serApi       = nullptr;  ///< REST API 封装
    DownloadManager*   m_downloadMgr  = nullptr;  ///< 文件传输管理
    TestOrchestrator*  m_testOrch     = nullptr;  ///< 测试流程状态机
    NetworkMonitor*    m_netMonitor   = nullptr;  ///< 网络监控
    SystemTrayManager* m_sysTray      = nullptr;  ///< 系统托盘

    InitialPage*       m_initialPage  = nullptr;  ///< 初始化页面
    QVector<StageTestPage*> m_stagePages;         ///< 各阶段测试页面

    DeviceInfo         m_deviceInfo;   ///< 当前设备信息
    TestPlanInfo       m_testPlan;     ///< 当前测试计划

    Ui::MainWindow     ui;

signals:
    /** @brief 服务端查询结果（后台线程发射，MainWindow 主线程接收） */
    void serverQueryResult(const DeviceInfo& deviceInfo,
                           const DeviceBaseDataInfo& devInfo,
                           const TestPlanInfo& plan);
    void routeInfoReady(const QString& processName);

private slots:
    void onDeviceInfoReady(const DeviceInfo& info);   ///< 设备采集成功
    void onDeviceInfoError(const QString& error);     ///< 设备采集失败
    void onTestStageStarted(int stageIndex, const QString& stageName);  ///< 测试阶段开始
    void onTestItemStarted(int stageIndex, int itemIndex, const QString& itemName);  ///< 测试项开始
    void onTestItemCompleted(int stageIndex, int itemIndex, int result);  ///< 测试项完成
    void onTestStageCompleted(int stageIndex, bool success);  ///< 测试阶段完成
    void onAllTestsCompleted(bool success);           ///< 所有测试完成
    void onTestError(const QString& errorMsg);        ///< 测试错误
    void onDownloadProgress(const QString& url, qint64 received, qint64 total);  ///< 下载进度
    void onBlueScreenDetected(const QString& message);  ///< 蓝屏检测
    void onTabChanged(int index);                     ///< Tab 切换

    /** @brief 服务端查询结果回调（后台线程信号 → 主线程槽） */
    void onServerQueryDone(const DeviceInfo& deviceInfo,
                           const DeviceBaseDataInfo& devInfo,
                           const TestPlanInfo& plan);
    /** @brief 工序信息回调（更新 UI 工序标签） */
    void onRouteInfoReady(const QString& processName);
};
