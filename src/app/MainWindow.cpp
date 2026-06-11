#include "MainWindow.h"
#include "qtlogger.h"
#include "ConfigManager.h"

// 业务模块
#include "DeviceModel.h"
#include "SerApiModel.h"
#include "DownloadManager.h"
#include "TestOrchestrator.h"
#include "NetworkMonitor.h"
#include "SystemTrayManager.h"
#include "SettingsAction.h"
#include "BindDeviceAction.h"
#include "SettingsDialog.h"

// ── UI 组件 ──
#include "EftpTabWidget.h"
#include "InitialPage.h"
#include "StageTestPage.h"
#include "ErrorDialog.h"
#include "SNMacBindDialog.h"

#include <QtConcurrent>

MainWindow::MainWindow(const QString& env, const QString& sn, QWidget* parent)
    : QMainWindow(parent)
    , m_env(env)
    , m_sn(sn)
{
    // 注册自定义类型（跨线程信号/槽需要）
    qRegisterMetaType<DeviceInfo>("DeviceInfo");
    qRegisterMetaType<DeviceBaseDataInfo>("DeviceBaseDataInfo");
    qRegisterMetaType<TestPlanInfo>("TestPlanInfo");

    ui.setupUi(this);
    setWindowTitle(QString("EFTP - %1 [%2]").arg(sn.isEmpty() ? "等待扫描" : sn).arg(env));

    // 设置主布局拉伸因子：标题栏(0) + 设备信息(0) + Tab区域(1) + Tips(0)
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
    if (mainLayout) {
        mainLayout->setStretch(0, 0); // 标题栏
        mainLayout->setStretch(1, 0); // 设备信息
        mainLayout->setStretch(2, 1); // Tab 区域（占据剩余空间）
        mainLayout->setStretch(3, 0); // Tips 区域（固定高度）
    }

    createModules();
    connectSignals();
    startWorkflow();

    // 初始化系统托盘
    m_sysTray->init(":/Eftp/icon/e.ico");
}

MainWindow::~MainWindow()
{
}

void MainWindow::createModules()
{
    m_deviceModel = new DeviceModel(this);
    m_serApi      = new SerApiModel();
    m_downloadMgr = new DownloadManager(this);
    m_testOrch    = new TestOrchestrator(this);
    m_netMonitor  = new NetworkMonitor(this);
    m_sysTray     = new SystemTrayManager(this);

    // 设置 HTTP 重试回调（更新 Tips 提示）
    SerApiModel::setRetryCallback([this](int retryCount, const std::string& url) {
        QMetaObject::invokeMethod(this, [this, retryCount]() {
            ui.labelTitleResult->setText(QString("网络请求重试中... (第 %1 次)").arg(retryCount));
            ui.textEditTips->setText(QString("网络请求失败，正在重试 (第 %1 次)").arg(retryCount));
        }, Qt::QueuedConnection);
    });

    // 注册托盘菜单动作
    auto* settingsAction = new SettingsAction(m_sysTray);
    connect(settingsAction, &SettingsAction::openSettings, this, [this]() {
        SettingsDialog dlg(this);
        dlg.exec();
    });
    m_sysTray->addAction(settingsAction);

    // 设备绑定（手动触发，防止误操作）
    auto* bindAction = new BindDeviceAction(m_sysTray);
    connect(bindAction, &BindDeviceAction::openBindDialog, this, [this]() {
        SNMacBindDialog dlg(m_deviceInfo.deviceMac, this);
        if (dlg.exec() == QDialog::Accepted) {
            QString sn = dlg.sn();
            QString mac = dlg.mac();
            QtLogger::WriteLog("MainWindow: 设备绑定 SN=" + sn + " MAC=" + mac);
            // 后台发送绑定请求
            QtConcurrent::run([this, sn, mac]() {
                SerApiModel api;
                api.addMacAddr(sn.toStdString(), mac.toStdString(),
                    m_deviceInfo.baseboardSn.toStdString(), 1);
                QtLogger::WriteLog("MainWindow: 设备绑定请求已发送");
            });
        }
    });
    m_sysTray->addAction(bindAction);
}

void MainWindow::connectSignals()
{
    // 设备采集
    connect(m_deviceModel, &DeviceModel::deviceInfoReady, this, &MainWindow::onDeviceInfoReady);
    connect(m_deviceModel, &DeviceModel::deviceInfoError, this, &MainWindow::onDeviceInfoError);

    // TestOrchestrator ←→ DownloadManager
    connect(m_testOrch, &TestOrchestrator::requestDownload,
            m_downloadMgr, &DownloadManager::onRequestDownload);
    connect(m_downloadMgr, &DownloadManager::downloadCompleted,
            m_testOrch, &TestOrchestrator::onDownloadCompleted);

    // 下载状态 → Tips + 标题栏
    connect(m_testOrch, &TestOrchestrator::requestDownload,
            this, [this](int, const QString& url, const QString&) {
                ui.labelTitleResult->setText("准备下载: " + url);
                ui.textEditTips->setText("下载中: " + url);
            });
    connect(m_downloadMgr, &DownloadManager::downloadCompleted,
            this, [this](int, const QString& localPath, bool success) {
                ui.textEditTips->setText(success
                    ? "下载完成: " + localPath
                    : "下载失败: " + localPath);
            });

    // 下载进度 → Tips + 标题栏显示（跨线程，用 QueuedConnection）
    connect(m_downloadMgr, &DownloadManager::downloadProgress,
            this, [this](const QString&, qint64 received, qint64 total) {
                if (total > 0) {
                    int percent = (int)(received * 100 / total);
                    double recvMB = (double)received / 1024.0 / 1024.0;
                    double totalMB = (double)total / 1024.0 / 1024.0;
                    QString sizeStr = QString("%1MB/%2MB").arg(recvMB, 0, 'f', 1).arg(totalMB, 0, 'f', 1);
                    ui.labelTitleResult->setText(QString("下载中 %1% (%2)").arg(percent).arg(sizeStr));
                    ui.textEditTips->setText(QString("下载进度: %1% (%2)").arg(percent).arg(sizeStr));
                }
            }, Qt::QueuedConnection);

    // ═══ 后台版本检查（TestOrchestrator 内置）═══
    // TestOrchestrator 请求下载 → DownloadManager 执行
    // (requestDownload 信号已在上方连接)

    // 测试项状态跟踪 → TestOrchestrator 跳过正在测试的项
    connect(m_testOrch, &TestOrchestrator::testItemStarted,
            this, [this](int stage, int item, const QString&) {
                m_testOrch->setTestingItem(stage, item);
            });
    connect(m_testOrch, &TestOrchestrator::testItemCompleted,
            this, [this](int, int, int) {
                m_testOrch->clearTestingItem();
            });

    // ═══ NetworkMonitor 信号 ═══
    connect(m_netMonitor, &NetworkMonitor::networkLost, this, [this]() {
        ui.textEditTips->setText("网络连接异常，正在重试...");
        QtLogger::WriteLog("MainWindow: 网络断开");
    });
    connect(m_netMonitor, &NetworkMonitor::networkRestored, this, [this]() {
        ui.textEditTips->setText("网络已恢复");
        QtLogger::WriteLog("MainWindow: 网络恢复");
    });
    connect(m_netMonitor, &NetworkMonitor::wifiSwitching, this, [this](const QString& ssid) {
        ui.labelTitleResult->setText("正在切换WiFi: " + ssid);
    });
    connect(m_netMonitor, &NetworkMonitor::wifiConnected, this, [this](const QString& ssid) {
        ui.labelTitleResult->setText("WiFi已连接: " + ssid);
    });

    // WiFi 手动/自动切换按钮
    connect(ui.btnWifiMode, &QPushButton::clicked, this, &MainWindow::onWifiModeToggled);

    // 网络状态 → 右上角显示 + 延迟过高时双处提示
    connect(m_netMonitor, &NetworkMonitor::pingResult, this,
            [this](const QString& ssid, int latencyMs) {
        int threshold = ConfigManager::instance().latencyThresholdMs();
        if (latencyMs < 0) {
            ui.labelNetwork->setText(QString("%1 | 超时").arg(ssid));
            ui.labelNetwork->setStyleSheet("font-size:14px; color:#E73C31; font-weight:bold; padding:0 8px;");
            ui.labelTitleResult->setText("网络超时，请检查网络连接");
            ui.textEditTips->setText("网络超时: " + ssid);
        } else if (latencyMs > threshold) {
            ui.labelNetwork->setText(QString("%1 | %2ms 延迟过高").arg(ssid).arg(latencyMs));
            ui.labelNetwork->setStyleSheet("font-size:14px; color:#F37E00; padding:0 8px;");
            ui.labelTitleResult->setText(QString("网络延迟过高: %1ms").arg(latencyMs));
            ui.textEditTips->setText(QString("网络延迟过高: %1 %2ms (阈值%3ms)").arg(ssid).arg(latencyMs).arg(threshold));
        } else {
            ui.labelNetwork->setText(QString("%1 | %2ms").arg(ssid).arg(latencyMs));
            ui.labelNetwork->setStyleSheet("font-size:14px; color:#01B659; padding:0 8px;");
            // 网络正常，清除之前的异常提示
            if (ui.textEditTips->toPlainText().contains("延迟过高") || ui.textEditTips->toPlainText().contains("超时")) {
                ui.textEditTips->clear();
            }
            if (ui.labelTitleResult->text().contains("超时") || ui.labelTitleResult->text().contains("延迟过高")) {
                ui.labelTitleResult->clear();
            }
        }
    });

    // ═══ SystemTrayManager 信号（设置已由 SettingsAction 处理）═══
    connect(m_sysTray, &SystemTrayManager::showMainWindow, this, [this]() {
        showNormal();
        activateWindow();
    });
    connect(m_sysTray, &SystemTrayManager::exitApp, this, [this]() {
        qApp->quit();
    });

    // TestOrchestrator → UI
    connect(m_testOrch, &TestOrchestrator::testStarted, [this]() {
        ui.labelTitleResult->setText("测试进行中...");
    });
    connect(m_testOrch, &TestOrchestrator::showTips, this, [this](const QString& msg) {
        ui.labelTitleResult->setText(msg);
        ui.textEditTips->setText(msg);
    });
    connect(m_testOrch, &TestOrchestrator::testStageStarted, this, &MainWindow::onTestStageStarted);
    connect(m_testOrch, &TestOrchestrator::testItemStarted, this, &MainWindow::onTestItemStarted);
    connect(m_testOrch, &TestOrchestrator::testItemCompleted, this, &MainWindow::onTestItemCompleted);
    connect(m_testOrch, &TestOrchestrator::testStageCompleted, this, &MainWindow::onTestStageCompleted);
    connect(m_testOrch, &TestOrchestrator::allTestsCompleted, this, &MainWindow::onAllTestsCompleted);
    connect(m_testOrch, &TestOrchestrator::testError, this, &MainWindow::onTestError);

    // API 错误弹窗（非模态，成功后自动关闭）
    connect(this, &MainWindow::apiError, this, [this](const QString& msg) {
        if (m_errorDialog) {
            m_errorDialog->close();
            m_errorDialog->deleteLater();
            m_errorDialog = nullptr;
        }
        auto* dlg = new ErrorDialog(ErrorDialog::Error, "错误", msg, this);
        m_errorDialog = dlg;
        connect(dlg, &QDialog::finished, this, [this]() {
            m_errorDialog = nullptr;
        });
        dlg->show();
    }, Qt::QueuedConnection);

    // DownloadManager → UI
    connect(m_downloadMgr, &DownloadManager::downloadProgress, this, &MainWindow::onDownloadProgress);
    connect(m_downloadMgr, &DownloadManager::blueScreenDetected, this, &MainWindow::onBlueScreenDetected);

    // Tab 切换
    connect(ui.tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    // 服务端查询结果（跨线程信号 → 主线程槽）
    connect(this, &MainWindow::serverQueryResult, this, &MainWindow::onServerQueryDone);
    connect(this, &MainWindow::routeInfoReady, this, &MainWindow::onRouteInfoReady);
}

void MainWindow::startWorkflow()
{
    // 1. 连接默认 WiFi
    m_netMonitor->connectDefaultWifi();

    // 2. 启动网络监控
    m_netMonitor->startMonitor();

    // 3. 采集设备信息
    m_deviceModel->collectAsync();
}

/**
 * @brief 设备采集回调
 */
void MainWindow::onDeviceInfoReady(const DeviceInfo& info)
{
    m_deviceInfo = info;
    ui.labelSn->setText(info.deviceSn);

    m_downloadMgr->setDeviceInfo(info.deviceSn, info.deviceMac);

    // 后台查询设备信息 + 工序信息（立即显示）
    QtConcurrent::run([this, info]() {
        SerApiModel api;
        std::string sn = info.deviceSn.toStdString();

        QtLogger::WriteLog("MainWindow: 查询设备信息 SN=" + info.deviceSn);
        DeviceBaseDataInfo devInfo;
        int retry = 0;
        while (true) {
            devInfo = api.queryDeviceInfo(sn, info.deviceMac.toStdString(), info.baseboardSn.toStdString());
            if (devInfo.errorMessage.empty()) break;
            retry++;
            QtLogger::WriteLog(QString("MainWindow: queryDeviceInfo 失败，5秒后重试 (第 %1 次)").arg(retry));
            emit apiError(QString::fromStdString(devInfo.errorMessage));
            QThread::sleep(5);
        }

        // 查询工序信息（routeProcessesName 值不同）
        DeviceRouteDataInfo routeInfo = api.queryDeviceRouteInfo(sn, info.deviceMac.toStdString());

        // 根据工序连接对应 WiFi（手动模式下跳过）
        QMetaObject::invokeMethod(this, [this, routeInfo]() {
            if (m_autoWifiSwitch) {
                m_netMonitor->connectForRoute(routeInfo.routeProcessesId);
            } else {
                QtLogger::WriteLog("MainWindow: WiFi手动模式，跳过工序网络切换");
            }
        }, Qt::QueuedConnection);

        QtLogger::WriteLog(QString("MainWindow: 设备信息完成, 工序=%1, 工作站=%2, routeProcessesId=%3")
            .arg(QString::fromUtf8(routeInfo.routeProcessesName.c_str()))
            .arg(QString::fromUtf8(devInfo.routeProcessesName.c_str()))
            .arg(routeInfo.routeProcessesId));

        // 发射设备信息信号（不覆盖 routeProcessesName）
        TestPlanInfo emptyPlan;
        emit serverQueryResult(info, devInfo, emptyPlan);
        emit routeInfoReady(QString::fromUtf8(routeInfo.routeProcessesName.c_str()));
    });

    // 后台查询测试计划（独立，不阻塞设备信息显示）
    QtConcurrent::run([this, info]() {
        SerApiModel api;
        std::string sn = info.deviceSn.toStdString();

        QtLogger::WriteLog("MainWindow: 查询测试计划...");
        TestPlanInfo plan;
        int retry2 = 0;
        while (true) {
            plan = api.queryDeviceTestInfo(sn);
            if (plan.errorMessage.empty()) break;
            retry2++;
            QtLogger::WriteLog(QString("MainWindow: queryDeviceTestInfo 失败，5秒后重试 (第 %1 次)").arg(retry2));
            emit apiError(QString::fromStdString(plan.errorMessage));
            QThread::sleep(5);
        }
        QtLogger::WriteLog(QString("MainWindow: 查询测试计划完成, stages=%1, isAuto=%2")
            .arg((int)plan.stages.size()).arg(plan.isAutoExecute));

        // API 成功，关闭错误弹窗
        QMetaObject::invokeMethod(this, [this]() {
            if (m_errorDialog) {
                m_errorDialog->close();
                m_errorDialog->deleteLater();
                m_errorDialog = nullptr;
            }
        }, Qt::QueuedConnection);

        // 发射测试计划信号
        DeviceBaseDataInfo emptyDevInfo;
        emit serverQueryResult(info, emptyDevInfo, plan);
    });
}

void MainWindow::onDeviceInfoError(const QString& error)
{
    ui.labelTitleResult->setText("采集失败: " + error);
    ui.textEditTips->setText("设备信息采集失败: " + error);
}

/**
 * @brief 服务端查询结果回调（主线程）
 *        接收设备信息和测试计划信号，更新 UI
 */
void MainWindow::onServerQueryDone(const DeviceInfo& deviceInfo,
                                    const DeviceBaseDataInfo& devInfo,
                                    const TestPlanInfo& plan)
{
    QtLogger::WriteLog("MainWindow: onServerQueryDone 执行");

    // 更新设备信息（如果有）
    if (!devInfo.spuName.empty()) {
        ui.labelSn->setText(deviceInfo.deviceSn);
        ui.labelDeviceType->setText(QString::fromUtf8(devInfo.spuName.c_str()));
        ui.labelArea->setText(QString::fromUtf8(devInfo.areaName.c_str()));
        ui.labelWorkStation->setText(QString::fromUtf8(devInfo.routeProcessesName.c_str()));
        ui.labelWorkOrder->setText(QString::fromUtf8(devInfo.workOrderNo.c_str()));
        ui.labelCurrentSKU->setText(QString::fromUtf8(devInfo.skuCode.c_str()) + " " + QString::fromUtf8(devInfo.skuName.c_str()));
        ui.labelAimSKU->setText(QString::fromUtf8(devInfo.targetSku.c_str()) + " " + QString::fromUtf8(devInfo.targetSkuName.c_str()));
        setWindowTitle(QString("EFTP - %1 [%2] %3")
            .arg(deviceInfo.deviceSn, m_env, QString::fromUtf8(devInfo.skuName.c_str())));
        QtLogger::WriteLog("MainWindow: 设备信息已更新");
    }

    // 更新测试计划（如果有）
    if (!plan.stages.empty()) {
        m_testPlan = plan;
        createTabPages(plan);
        m_testOrch->setDeviceSn(deviceInfo.deviceSn.toStdString());
        m_testOrch->setTestPlan(plan);

        // 等用户手动点击"开始测试"按钮
        ui.labelTitleResult->setText(u8"请在对应阶段点击\"开始测试\"");

        QtLogger::WriteLog("MainWindow: 测试计划已更新");
    }

    m_downloadMgr->checkAndUpload();
    QtLogger::WriteLog("MainWindow: UI 更新完成");
}

/**
 * @brief 创建 Tab 页面（初始化页 + 各测试阶段页）
 */
void MainWindow::createTabPages(const TestPlanInfo& plan)
{
    ui.tabWidget->clear();
    m_stagePages.clear();
    m_initialPage = nullptr; // 重置，避免残留指针导致 tabIndex 偏移

    if (plan.isInit == 1) {
        m_initialPage = new InitialPage(this);
        m_initialPage->setInitInfo(plan.initInfo);
        ui.tabWidget->addTab(m_initialPage, "初始化");
        connect(m_initialPage, &InitialPage::initCompleted, [this]() {
            if (!m_stagePages.isEmpty())
                ui.tabWidget->setCurrentIndex(1);
        });
    }

    for (int i = 0; i < (int)plan.stages.size(); i++) {
        StageTestPage* page = new StageTestPage(this);
        page->setStageData(plan.stages[i], plan.stages[i].isAutoExecute == 1);
        ui.tabWidget->addTab(page, QString::fromUtf8(plan.stages[i].stageName.c_str()));
        m_stagePages.append(page);

        // 连接"开始测试"按钮 → TestOrchestrator
        connect(page, &StageTestPage::startTestRequested, [this, i]() {
            m_testOrch->start(i);
        });

        // 连接"重测"按钮 → TestOrchestrator
        connect(page, &StageTestPage::retestRequested, [this](int cycleItemId) {
            m_testOrch->retest(cycleItemId);
        });
    }
}

/**
 * @brief 测试流程回调（Tab 切换、测试项状态更新）
 */
void MainWindow::onTestStageStarted(int stageIndex, const QString& stageName)
{
    ui.labelTitleResult->setText(QString("阶段 %1: %2").arg(stageIndex + 1).arg(stageName));
    int tabIndex = stageIndex + (m_initialPage ? 1 : 0);
    QtLogger::WriteLog(QString("MainWindow: onTestStageStarted stage=%1, tabIndex=%2").arg(stageIndex).arg(tabIndex));

    if (tabIndex < ui.tabWidget->count()) {
        ui.tabWidget->setCurrentIndex(tabIndex);
        EftpTabWidget* tabWidget = qobject_cast<EftpTabWidget*>(ui.tabWidget);
        if (tabWidget) {
            tabWidget->setTabTestState(tabIndex, TestState::Running);
            QtLogger::WriteLog(QString("MainWindow: Tab %1 设为 Running").arg(tabIndex));
        } else {
            QtLogger::WriteLog("MainWindow: qobject_cast<EftpTabWidget*> 失败!");
        }
    }
}

void MainWindow::onTestItemStarted(int stageIndex, int itemIndex, const QString& itemName)
{
    Q_UNUSED(itemName);
    if (stageIndex >= 0 && stageIndex < m_stagePages.size())
        m_stagePages[stageIndex]->setItemRunning(itemIndex);

    int tabIndex = stageIndex + (m_initialPage ? 1 : 0);
    EftpTabWidget* tabWidget = qobject_cast<EftpTabWidget*>(ui.tabWidget);
    if (tabWidget && tabIndex < ui.tabWidget->count()) {
        tabWidget->setTabTestState(tabIndex, TestState::Running);
        QtLogger::WriteLog(QString("MainWindow: Tab %1 设为 Running (itemStarted)").arg(tabIndex));
    }
}

void MainWindow::onTestItemCompleted(int stageIndex, int itemIndex, int result)
{
    if (stageIndex >= 0 && stageIndex < m_stagePages.size())
        m_stagePages[stageIndex]->updateItemResult(itemIndex, result);

    QtLogger::WriteLog(QString("MainWindow: onTestItemCompleted stage=%1, item=%2, result=%3")
        .arg(stageIndex).arg(itemIndex).arg(result));

    int tabIndex = stageIndex + (m_initialPage ? 1 : 0);
    EftpTabWidget* tabWidget = qobject_cast<EftpTabWidget*>(ui.tabWidget);
    if (!tabWidget || tabIndex >= ui.tabWidget->count()) {
        QtLogger::WriteLog("MainWindow: tabWidget 无效或 tabIndex 超出范围");
        return;
    }

    // 检查该阶段所有测试项状态
    bool hasFail = false;
    bool allDone = true;
    if (stageIndex >= 0 && stageIndex < (int)m_testPlan.stages.size()) {
        const auto& stage = m_testPlan.stages[stageIndex];
        for (int i = 0; i < (int)stage.items.size(); i++) {
            int row = i / 8;
            int col = i % 8;
            QModelIndex idx = m_stagePages[stageIndex]->getModel()->index(row, col);
            int state = idx.data(ItemProgramTestModel::ProgramStateRole).toInt();
            QtLogger::WriteLog(QString("  项%1: state=%2").arg(i).arg(state));
            if (state == static_cast<int>(TestState::Fail))
                hasFail = true;
            if (state == static_cast<int>(TestState::UnStart) || state == static_cast<int>(TestState::Running))
                allDone = false;
        }
    }

    QtLogger::WriteLog(QString("MainWindow: 阶段%1 allDone=%2, hasFail=%3").arg(stageIndex).arg(allDone).arg(hasFail));

    if (allDone) {
        TestState tabState = hasFail ? TestState::Fail : TestState::Success;
        tabWidget->setTabTestState(tabIndex, tabState);
        QtLogger::WriteLog(QString("MainWindow: Tab %1 设为 %2").arg(tabIndex).arg(hasFail ? "Fail" : "Success"));
    }
}

void MainWindow::onTestStageCompleted(int stageIndex, bool success)
{
    // Tab 颜色由 onTestItemCompleted 根据实际结果设置，这里不覆盖
    Q_UNUSED(stageIndex);
    Q_UNUSED(success);
}

void MainWindow::onAllTestsCompleted(bool success)
{
    ui.labelTitleResult->setText(success ? "所有测试完成 ✓" : "测试完成（有失败项）");
    QString resultStyle = success
        ? "border-image:url(:/Eftp/image/success2x.png);"
        : "border-image:url(:/Eftp/image/testFail2x.png);";
    ui.labelResult->setStyleSheet(resultStyle);
}

void MainWindow::onTestError(const QString& errorMsg)
{
    ui.labelTitleResult->setText(errorMsg);
    ui.textEditTips->setText(errorMsg);
}

void MainWindow::onDownloadProgress(const QString& url, qint64 received, qint64 total)
{
    Q_UNUSED(url);
    Q_UNUSED(received);
    Q_UNUSED(total);
    // 方案 A：不显示实时进度，只显示下载状态
}

void MainWindow::onBlueScreenDetected(const QString& message)
{
    ui.labelTitleResult->setText(message);
    ui.labelResult->setStyleSheet("border-image:url(:/Eftp/image/testFail2x.png);");
    ui.textEditTips->setText(message);
}

void MainWindow::onTabChanged(int index)
{
    Q_UNUSED(index);
}

void MainWindow::onRouteInfoReady(const QString& processName)
{
    ui.labelProcessName->setText(processName);
    QtLogger::WriteLog("MainWindow: 工序信息已更新: " + processName);
}

void MainWindow::onWifiModeToggled()
{
    m_autoWifiSwitch = !m_autoWifiSwitch;
    if (m_autoWifiSwitch) {
        ui.btnWifiMode->setText("自动");
        ui.btnWifiMode->setProperty("mode", "");
        ui.btnWifiMode->style()->unpolish(ui.btnWifiMode);
        ui.btnWifiMode->style()->polish(ui.btnWifiMode);
        ui.textEditTips->setText("WiFi已切换为自动模式");
        QtLogger::WriteLog("MainWindow: WiFi切换为自动模式");
    } else {
        ui.btnWifiMode->setText("手动");
        ui.btnWifiMode->setProperty("mode", "manual");
        ui.btnWifiMode->style()->unpolish(ui.btnWifiMode);
        ui.btnWifiMode->style()->polish(ui.btnWifiMode);
        ui.textEditTips->setText("WiFi已切换为手动模式，不会随工序自动切换网络");
        QtLogger::WriteLog("MainWindow: WiFi切换为手动模式");
    }
}
