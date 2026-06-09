#pragma once

/**
 * @file DownloadManager.h
 * @brief 文件传输管理器（下载/上传/蓝屏Dump）— SHARED DLL
 *
 * 功能：
 *   - 文件下载 + 7z 解压（测试程序更新）
 *   - 文件上传（multipart/form-data）
 *   - 蓝屏 Dump 检测、压缩、上传、去重
 *
 * 业务逻辑：
 *   下载流程：
 *     1. TestOrchestrator 发射 requestDownload 信号
 *     2. MainWindow 路由到 onRequestDownload 槽
 *     3. 后台线程执行 downloadAndExtract（HTTP GET + 7za 解压）
 *     4. 完成后发射 downloadCompleted 信号
 *
 *   蓝屏 Dump 流程：
 *     1. 检测 C:\Windows\MEMORY.DMP（全内存转储）
 *     2. 检测 C:\Windows\Minidump\ 目录
 *     3. 检测 Windows 事件日志中的意外关机事件（Event ID 41）
 *     4. 压缩 → 上传 → 上报元数据（addExtData）
 *     5. 通过日志文件去重，避免重复上传
 *
 * SHARED 理由：传输策略可变，DLL 可独立替换
 */

#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include "EftpTypes.h"

#include <QtCore/qglobal.h>

#ifdef DOWNLOADMANAGER_LIBRARY
#  define DOWNLOADMANAGER_EXPORT Q_DECL_EXPORT
#else
#  define DOWNLOADMANAGER_EXPORT Q_DECL_IMPORT
#endif

class DOWNLOADMANAGER_EXPORT DownloadManager : public QObject
{
    Q_OBJECT

public:
    explicit DownloadManager(QObject* parent = nullptr);
    ~DownloadManager();

    // ═══════════════════════════════════════════
    // 下载
    // ═══════════════════════════════════════════

    bool downloadAndExtract(const QString& url, const QString& localPath);
    QString lastError() const { return m_lastError; }

public slots:
    void onRequestDownload(int requestId, const QString& url, const QString& localPath);

    // ═══════════════════════════════════════════
    // 上传
    // ═══════════════════════════════════════════

    /**
     * @brief 上传文件到服务器
     * @param fullUrl 上传地址
     * @param filePath 本地文件路径
     * @return 服务器返回的文件 URL，失败返回空串
     */
    std::string uploadFile(const std::string& fullUrl, const std::string& filePath);

    // ═══════════════════════════════════════════
    // 蓝屏 Dump
    // ═══════════════════════════════════════════

    void setDeviceInfo(const QString& sn, const QString& mac);
    void checkAndUpload();

signals:
    /** @brief 下载进度（MainWindow 更新 UI） */
    void downloadProgress(const QString& url, qint64 bytesReceived, qint64 bytesTotal);
    /** @brief 下载完成（MainWindow 路由到 TestOrchestrator） */
    void downloadCompleted(int requestId, const QString& localPath, bool success);

    /** @brief 文件上传完成 */
    void uploadCompleted(const std::string& fileUrl, bool success);

    /** @brief 检测到 dump 文件（MainWindow 更新 UI） */
    void dumpFound(const QString& filePath, const QString& type);
    /** @brief dump 上传完成 */
    void dumpUploaded(const QString& fileUrl, bool success);
    /** @brief 检测到蓝屏文件（MainWindow 显示警告） */
    void blueScreenDetected(const QString& message);

private:
    // ── 下载 ──
    bool downloadFile(const QString& url, const QString& savePath);
    bool extractBy7za(const QString& archivePath, const QString& extractDir);

    // ── 上传 ──
    std::string doUploadFile(const std::string& fullUrl, const std::string& filePath);

    // ── Dump 检测 ──
    void checkMemoryDump();
    void checkMinidump();
    void checkUnexpectedShutdownEvents();

    // ── Dump 上传 ──
    void uploadMemoryDump(const QString& path);
    void uploadMinidumpFiles(const QFileInfoList& files);
    QString compressDumpFile(const QString& sourcePath);
    void postDumpMetadata(const UploadDumpFileInfo& info);

    // ── 去重 ──
    bool isAlreadyUploaded(const QString& logPath, const QString& key);
    void markAsUploaded(const QString& logPath, const QString& key);

    // ── 成员 ──
    QNetworkAccessManager m_networkMgr;
    QString m_sn;
    QString m_mac;
    QString m_lastError;  // 最后一次失败原因

    static const QString DUMP_LOG_PATH;
    static const QString SHUTDOWN_LOG_PATH;
};
