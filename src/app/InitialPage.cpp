#include "InitialPage.h"
#include "DownloadManager.h"
#include "qtlogger.h"
#include <QDir>
#include <QProcess>
#include <QtConcurrent>

InitialPage::InitialPage(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    // 左侧：全部测试程序列表
    ui.treeWidgetAll->setColumnWidth(0, 180);
    ui.treeWidgetAll->setColumnWidth(1, 80);
    ui.treeWidgetAll->header()->setStretchLastSection(true);

    // 右侧：失败项列表
    ui.treeWidgetFail->setColumnWidth(0, 200);
    ui.treeWidgetFail->header()->setStretchLastSection(true);

    connect(ui.btnStart, &QPushButton::clicked, this, &InitialPage::onStartClicked);
    connect(ui.btnRetryAll, &QPushButton::clicked, this, &InitialPage::onRetryAllClicked);
    connect(this, &InitialPage::clearFileDone, this, &InitialPage::onClearFileDone, Qt::QueuedConnection);
    connect(this, &InitialPage::downloadItemDone, this, &InitialPage::onDownloadItemDone, Qt::QueuedConnection);
    connect(ui.btnRetryAll, &QPushButton::clicked, this, &InitialPage::onRetryAllClicked);

    // 失败列表点击重试
    connect(ui.treeWidgetFail, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item, int column) {
        if (column == 1 && item) {
            onRetryClicked(item);
        }
    });
}

InitialPage::~InitialPage()
{
}

void InitialPage::setInitInfo(const TestDeviceInitInfoVO& initInfo)
{
    m_initInfo = initInfo;

    // 填充左侧全部测试程序列表
    ui.treeWidgetAll->clear();
    for (int i = 0; i < initInfo.testItemVOList.size(); i++) {
        const auto& item = initInfo.testItemVOList[i];
        QTreeWidgetItem* treeItem = new QTreeWidgetItem(ui.treeWidgetAll);
        treeItem->setText(0, item.itemName);
        treeItem->setText(1, "待处理");
        treeItem->setText(2, item.unzipPath);
        treeItem->setForeground(1, QColor("#8D8D8D"));
        treeItem->setData(0, Qt::UserRole, i); // 保存索引
    }

    ui.labelProgress->setText(QString("共 %1 个测试程序").arg(initInfo.testItemVOList.size()));

    // 自动触发初始化（延迟 100ms 确保 UI 渲染完成）
    QTimer::singleShot(100, this, &InitialPage::onStartClicked);
}

/**
 * @brief 开始初始化
 */
void InitialPage::onStartClicked()
{
    if (m_isRunning) return;
    m_isRunning = true;
    m_clearDoneCount = 0;
    m_doneCount = 0;
    m_failCount = 0;

    ui.btnStart->setEnabled(false);
    ui.btnStart->setText("初始化中...");
    ui.labelProgress->setText("正在清除旧测试文件...");

    startClearFiles();
}

/**
 * @brief 步骤 1：清除旧测试文件目录
 */
void InitialPage::startClearFiles()
{
    m_clearDoneCount = 0;
    int total = m_initInfo.testItemVOList.size();
    QtLogger::WriteLog(QString("InitialPage: 开始清除旧文件, 共 %1 个").arg(total));

    for (int i = 0; i < total; i++) {
        int idx = i;
        QString unzipPath = m_initInfo.testItemVOList[i].unzipPath;

        QtConcurrent::run([this, idx, unzipPath]() {
            bool success = clearDirectory(unzipPath);
            emit clearFileDone(idx, success);
        });
    }
}

bool InitialPage::clearDirectory(const QString& path)
{
    QDir dir(path);
    if (!dir.exists()) return true;
    return dir.removeRecursively();
}

void InitialPage::onClearFileDone(int index, bool success)
{
    updateAllStatus(index, success ? "已清除" : "清除失败", success ? "#01B659" : "#E73C31");
    m_clearDoneCount++;

    if (m_clearDoneCount >= m_initInfo.testItemVOList.size()) {
        QtLogger::WriteLog("InitialPage: 全部清除完成");
        startDefenderExclusion();
    }
}

/**
 * @brief 步骤 2：添加 Windows Defender 排除路径
 */
void InitialPage::startDefenderExclusion()
{
    QString exclusionPath = m_initInfo.dirWhiteList;
    if (exclusionPath.isEmpty()) {
        startDownloadFiles();
        return;
    }

    QtConcurrent::run([this, exclusionPath]() {
        QProcess process;
        process.start("powershell", {"-Command",
            QString("Add-MpPreference -ExclusionPath '%1'").arg(exclusionPath)});
        process.waitForFinished(10000);
        bool success = (process.exitCode() == 0);
        QMetaObject::invokeMethod(this, [this, success]() {
            onDefenderDone(success);
        }, Qt::QueuedConnection);
    });
}

void InitialPage::onDefenderDone(bool success)
{
    QtLogger::WriteLog(QString("InitialPage: Defender 排除 %1").arg(success ? "成功" : "失败"));
    startDownloadFiles();
}

/**
 * @brief 步骤 3：下载测试程序并解压
 */
void InitialPage::startDownloadFiles()
{
    m_currentIndex = 0;
    ui.labelProgress->setText("正在下载测试程序...");
    downloadNext();
}

void InitialPage::downloadNext()
{
    // 跳过已存在的
    while (m_currentIndex < m_initInfo.testItemVOList.size()) {
        QTreeWidgetItem* item = ui.treeWidgetAll->topLevelItem(m_currentIndex);
        if (item && item->text(1) == "跳过") {
            m_currentIndex++;
            continue;
        }
        break;
    }

    if (m_currentIndex >= m_initInfo.testItemVOList.size()) {
        onAllDone();
        return;
    }

    const auto& item = m_initInfo.testItemVOList[m_currentIndex];
    updateAllStatus(m_currentIndex, "下载中...", "#F37E00");
    ui.labelProgress->setText(QString("正在下载: %1").arg(item.itemName));

    int idx = m_currentIndex;
    QString downLoadPath = item.downLoadPath;
    QString unzipPath = item.unzipPath;

    QtConcurrent::run([this, idx, downLoadPath, unzipPath]() {
        DownloadManager mgr;
        bool success = mgr.downloadAndExtract(downLoadPath, unzipPath);
        emit downloadItemDone(idx, success);
    });
}

void InitialPage::onDownloadItemDone(int index, bool success, const QString& reason)
{
    if (success) {
        updateAllStatus(index, "完成", "#01B659");
        removeFailedItem(index);
        m_doneCount++;
        m_retriedItems.remove(index);
    } else {
        // 自动重试一次（如果还没重试过）
        if (!m_retriedItems.contains(index)) {
            m_retriedItems.insert(index);
            updateAllStatus(index, "重试中...", "#F37E00");
            QtLogger::WriteLog(QString("InitialPage: 自动重试 %1").arg(m_initInfo.testItemVOList[index].itemName));

            const auto& item = m_initInfo.testItemVOList[index];
            int idx = index;
            QString downLoadPath = item.downLoadPath;
            QString unzipPath = item.unzipPath;
            QtConcurrent::run([this, idx, downLoadPath, unzipPath]() {
                DownloadManager mgr;
                bool ok = mgr.downloadAndExtract(downLoadPath, unzipPath);
                emit downloadItemDone(idx, ok, ok ? "" : mgr.lastError());
            });
            return; // 不移动到下一个，等重试结果
        }

        // 重试后仍然失败
        updateAllStatus(index, "失败", "#E73C31");
        addFailedItem(index, reason);
        m_failCount++;
        m_retriedItems.remove(index);
    }

    m_currentIndex++;
    int total = m_initInfo.testItemVOList.size();
    ui.labelProgress->setText(QString("进度: %1/%2").arg(m_doneCount + m_failCount).arg(total));

    downloadNext();
}

/**
 * @brief 失败项管理（添加/移除/重试）
 */
void InitialPage::addFailedItem(int index, const QString& reason)
{
    const auto& item = m_initInfo.testItemVOList[index];

    // 检查是否已在失败列表中
    for (int i = 0; i < ui.treeWidgetFail->topLevelItemCount(); i++) {
        if (ui.treeWidgetFail->topLevelItem(i)->data(0, Qt::UserRole).toInt() == index) {
            return; // 已存在
        }
    }

    QTreeWidgetItem* failItem = new QTreeWidgetItem(ui.treeWidgetFail);
    failItem->setText(0, item.itemName);
    failItem->setText(1, "重新下载");
    failItem->setData(0, Qt::UserRole, index);
    failItem->setForeground(0, QColor("#E73C31"));

    // 动态显示失败原因
    if (!reason.isEmpty()) {
        ui.labelFailTip->setText(reason);
    } else {
        ui.labelFailTip->setText("下载失败");
    }
}

void InitialPage::removeFailedItem(int index)
{
    for (int i = 0; i < ui.treeWidgetFail->topLevelItemCount(); i++) {
        if (ui.treeWidgetFail->topLevelItem(i)->data(0, Qt::UserRole).toInt() == index) {
            delete ui.treeWidgetFail->topLevelItem(i);
            return;
        }
    }
}

void InitialPage::onRetryClicked(QTreeWidgetItem* item)
{
    int index = item->data(0, Qt::UserRole).toInt();
    if (index < 0 || index >= m_initInfo.testItemVOList.size()) return;

    const auto& testItem = m_initInfo.testItemVOList[index];
    updateAllStatus(index, "重新下载...", "#F37E00");
    removeFailedItem(index);

    int idx = index;
    QString downLoadPath = testItem.downLoadPath;
    QString unzipPath = testItem.unzipPath;

    QtConcurrent::run([this, idx, downLoadPath, unzipPath]() {
        DownloadManager mgr;
        bool success = mgr.downloadAndExtract(downLoadPath, unzipPath);
        emit downloadItemDone(idx, success);
    });
}

void InitialPage::onRetryAllClicked()
{
    // 收集所有失败项的索引
    QList<int> failedIndices;
    for (int i = 0; i < ui.treeWidgetFail->topLevelItemCount(); i++) {
        int index = ui.treeWidgetFail->topLevelItem(i)->data(0, Qt::UserRole).toInt();
        failedIndices.append(index);
    }

    if (failedIndices.isEmpty()) return;

    // 清空失败列表
    ui.treeWidgetFail->clear();
    m_failCount = 0;

    // 逐个重新下载
    for (int idx : failedIndices) {
        const auto& item = m_initInfo.testItemVOList[idx];
        updateAllStatus(idx, "重新下载...", "#F37E00");

        int index = idx;
        QString downLoadPath = item.downLoadPath;
        QString unzipPath = item.unzipPath;

        QtConcurrent::run([this, index, downLoadPath, unzipPath]() {
            DownloadManager mgr;
            bool success = mgr.downloadAndExtract(downLoadPath, unzipPath);
            emit downloadItemDone(index, success);
        });
    }

    ui.btnRetryAll->setEnabled(false);
    ui.btnRetryAll->setText("重试中...");
}

/**
 * @brief 初始化完成（更新 UI 状态，发射完成信号）
 */
void InitialPage::onAllDone()
{
    m_isRunning = false;
    ui.btnStart->setEnabled(true);
    ui.btnStart->setText("初始化完成");

    if (m_failCount > 0) {
        ui.labelProgress->setText(QString("完成，%1 个失败（请在右侧重试）").arg(m_failCount));
        ui.labelProgress->setStyleSheet("font-size:13px; color:#E73C31; font-weight:bold;");
    } else {
        ui.labelProgress->setText("全部完成 ✓");
        ui.labelProgress->setStyleSheet("font-size:13px; color:#01B659; font-weight:bold;");
        emit initCompleted();
    }

    QtLogger::WriteLog(QString("InitialPage: 初始化完成, 成功=%1, 失败=%2").arg(m_doneCount).arg(m_failCount));
}

void InitialPage::updateAllStatus(int index, const QString& status, const QString& color)
{
    QTreeWidgetItem* item = ui.treeWidgetAll->topLevelItem(index);
    if (item) {
        item->setText(1, status);
        item->setForeground(1, QColor(color));
    }
}
