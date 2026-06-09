#pragma once

#include <QWidget>
#include <QSet>
#include "ui_InitialPage.h"
#include "EftpTypes.h"

/**
 * @brief 初始化页面
 *
 * 左侧：全部测试程序列表 + 状态
 * 右侧：下载失败项 + 重新下载按钮
 *
 * 流程：
 *   1. 清除旧测试文件目录
 *   2. 添加 Windows Defender 排除路径
 *   3. 下载测试程序并解压
 *   4. 全部完成后发射 initCompleted 信号
 */
class InitialPage : public QWidget
{
    Q_OBJECT
public:
    explicit InitialPage(QWidget* parent = nullptr);
    ~InitialPage();

    void setInitInfo(const TestDeviceInitInfoVO& initInfo);

signals:
    void initCompleted();
    void initError(const QString& errorMsg);
    void clearFileDone(int index, bool success);
    void downloadItemDone(int index, bool success, const QString& reason = "");

public slots:
    void onClearFileDone(int index, bool success);
    void onDownloadItemDone(int index, bool success, const QString& reason = "");

private slots:
    void onStartClicked();
    void onRetryClicked(QTreeWidgetItem* item);
    void onRetryAllClicked();

private:
    // ── 流程步骤 ──
    void startClearFiles();
    void startDefenderExclusion();
    void onDefenderDone(bool success);
    void startDownloadFiles();
    void downloadNext();
    void onAllDone();

    // ── 辅助 ──
    void updateAllStatus(int index, const QString& status, const QString& color);
    void addFailedItem(int index, const QString& reason = "");
    void removeFailedItem(int index);
    bool clearDirectory(const QString& path);
    bool addDefenderExclusion(const QString& path);

    Ui::InitialPage ui;
    TestDeviceInitInfoVO m_initInfo;
    int m_currentIndex = 0;
    int m_doneCount = 0;
    int m_failCount = 0;
    bool m_isRunning = false;
    int m_clearDoneCount = 0;
    QSet<int> m_retriedItems;  // 已重试过的项目索引
};
