#include "DownloadManager.h"
#include "qtlogger.h"
#include "ConfigManager.h"
#include "SerApiModel.h"

#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrent>

#include <httplib.h>
#include <regex>
#include <fstream>

// Dump 去重日志路径
const QString DownloadManager::DUMP_LOG_PATH = "C:/Users/Public/Desktop/EDYTest/Tools/dumpFile.txt";
const QString DownloadManager::SHUTDOWN_LOG_PATH = "C:/Users/Public/Desktop/IllegalShutdownLog.log";

DownloadManager::DownloadManager(QObject* parent)
    : QObject(parent)
{
}

DownloadManager::~DownloadManager()
{
}

/**
 * @brief 下载并解压（同步阻塞，应在后台线程调用）
 *        下载 .7z → 7za 解压 → 删除压缩包
 */
bool DownloadManager::downloadAndExtract(const QString& url, const QString& localPath)
{
    m_lastError.clear();
    QtLogger::WriteLog("DownloadManager: 开始下载 " + url + " → " + localPath);

    // 1. 下载 .7z 文件
    QString archivePath = localPath + ".7z";
    if (!downloadFile(url, archivePath)) {
        m_lastError = "下载失败（网络错误或超时）";
        QtLogger::WriteLog("DownloadManager: 下载失败 " + url, enLogType::WARNING);
        return false;
    }
    QtLogger::WriteLog("DownloadManager: 下载成功 " + archivePath);

    // 2. 解压到目标目录
    QString extractDir = QFileInfo(localPath).absolutePath();
    if (!extractBy7za(archivePath, extractDir)) {
        m_lastError = "解压失败（压缩包损坏或7za超时）";
        QtLogger::WriteLog("DownloadManager: 解压失败 " + archivePath, enLogType::WARNING);
        return false;
    }
    QtLogger::WriteLog("DownloadManager: 解压成功 " + extractDir);

    return true;
}

/**
 * @brief 槽函数：接收下载请求（由 MainWindow 路由自 TestOrchestrator）
 */
void DownloadManager::onRequestDownload(int requestId, const QString& url, const QString& localPath)
{
    QtLogger::WriteLog(QString("DownloadManager: 收到下载请求 #%1").arg(requestId));

    // 后台线程执行下载
    QtConcurrent::run([=]() {
        bool success = downloadAndExtract(url, localPath);
        emit downloadCompleted(requestId, localPath, success);
    });
}

/**
 * @brief 下载文件（HTTP GET + 重试）
 *        进度超时：30 秒无新数据则认为卡住
 */
bool DownloadManager::downloadFile(const QString& url, const QString& savePath)
{
    int maxRetry = ConfigManager::instance().maxRetry();
    int progressTimeoutMs = 30000; // 30秒无数据视为卡住

    int retryCount = 0;

    while (retryCount < maxRetry) {
        QEventLoop loop;
        QTimer progressTimer; // 进度超时计时器
        bool finished = false;
        bool timeout = false;

        QNetworkRequest request{QUrl{url}};
        QNetworkReply* reply = m_networkMgr.get(request);

        // 每收到数据就重置计时器
        progressTimer.setSingleShot(true);
        QObject::connect(reply, &QNetworkReply::readyRead, &progressTimer, [&]() {
            progressTimer.start(progressTimeoutMs);
        });

        // 超时 → 标记超时并退出事件循环
        QObject::connect(&progressTimer, &QTimer::timeout, &loop, [&]() {
            timeout = true;
            loop.quit();
        });

        // 下载完成 → 退出事件循环
        QObject::connect(reply, &QNetworkReply::finished, &loop, [&]() {
            finished = true;
            loop.quit();
        });

        // 启动进度计时器（首次数据到达前用固定超时）
        progressTimer.start(progressTimeoutMs);
        loop.exec();

        if (timeout) {
            reply->abort();
            reply->deleteLater();
            retryCount++;
            QtLogger::WriteLog(QString("DownloadManager: 下载无进度（%1秒），第 %2 次重试")
                .arg(progressTimeoutMs / 1000).arg(retryCount));
            continue;
        }

        progressTimer.stop();

        if (reply->error() == QNetworkReply::NoError) {
            QFile file(savePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();
                reply->deleteLater();
                return true;
            } else {
                QtLogger::WriteLog("DownloadManager: 无法写入文件 " + savePath, enLogType::WARNING);
                reply->deleteLater();
                return false;
            }
        } else {
            QtLogger::WriteLog("DownloadManager: 下载错误 " + reply->errorString(), enLogType::WARNING);
            reply->deleteLater();
            retryCount++;
        }
    }

    QtLogger::WriteLog("DownloadManager: 超过最大重试次数 " + url, enLogType::SERIOUS);
    return false;
}

/**
 * @brief 7za 解压（调用 7za.exe 解压 .7z 文件）
 */
bool DownloadManager::extractBy7za(const QString& archivePath, const QString& extractDir)
{
    QString tool7za = QString::fromStdString(ConfigManager::instance().tool7zaPath());
    int extractTimeoutMs = ConfigManager::instance().extractTimeoutMs();

    QStringList arguments;
    arguments << "x" << archivePath
              << "-o" + extractDir
              << "-y";

    QProcess process;
    process.start(tool7za, arguments);

    if (!process.waitForFinished(extractTimeoutMs)) {
        process.kill();
        QtLogger::WriteLog("DownloadManager: 解压超时 " + archivePath, enLogType::WARNING);
        return false;
    }

    if (process.exitCode() != 0) {
        QString error = process.readAllStandardError();
        QtLogger::WriteLog("DownloadManager: 解压失败 " + error, enLogType::WARNING);
        return false;
    }

    // 删除 .7z 压缩包
    QFile::remove(archivePath);
    return true;
}

/**
 * @brief 上传文件（multipart/form-data）
 */
std::string DownloadManager::uploadFile(const std::string& fullUrl, const std::string& filePath)
{
    return doUploadFile(fullUrl, filePath);
}

std::string DownloadManager::doUploadFile(const std::string& fullUrl, const std::string& filePath)
{
    // 解析 URL
    std::regex urlRegex(R"(^(http|https)://([^/]+)(/.*)?$)");
    std::smatch match;
    if (!std::regex_search(fullUrl, match, urlRegex)) {
        QtLogger::WriteLog("DownloadManager: URL解析失败 " + QString::fromStdString(fullUrl), enLogType::WARNING);
        return "";
    }
    std::string host = match[1].str() + "://" + match[2].str();
    std::string path = match[3].str();
    if (path.empty()) path = "/";

    // 读取文件
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        QtLogger::WriteLog("DownloadManager: 无法打开文件 " + QString::fromStdString(filePath), enLogType::WARNING);
        return "";
    }
    std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // 提取文件名
    std::string filename = filePath;
    size_t pos = filename.find_last_of("/\\");
    if (pos != std::string::npos) filename = filename.substr(pos + 1);

    // 上传
    httplib::SSLClient cli(host);
    cli.set_connection_timeout(60);
    cli.set_read_timeout(60);

    httplib::MultipartFormDataItems items = {
        {"file", fileContent, filename, "application/octet-stream"}
    };

    auto res = cli.Post(path, items);
    if (res && res->status >= 200 && res->status < 300) {
        QtLogger::WriteLog("DownloadManager: 上传成功 " + QString::fromStdString(path));
        return res->body;
    }

    QtLogger::WriteLog("DownloadManager: 上传失败", enLogType::WARNING);
    return "";
}

/**
 * @brief Dump 检测入口（后台线程执行）
 *        检测 MEMORY.DMP + Minidump + 意外关机事件
 */
void DownloadManager::setDeviceInfo(const QString& sn, const QString& mac)
{
    m_sn = sn;
    m_mac = mac;
}

void DownloadManager::checkAndUpload()
{
    QtLogger::WriteLog("DownloadManager: 开始检测蓝屏文件");
    QtConcurrent::run([=]() {
        bool found = false;

        // MEMORY.DMP
        if (QFile::exists("C:/Windows/MEMORY.DMP")) {
            found = true;
            uploadMemoryDump("C:/Windows/MEMORY.DMP");
        }

        // Minidump
        QDir dir("C:/Windows/Minidump");
        if (dir.exists() && !dir.entryList(QDir::Files).isEmpty()) {
            found = true;
            uploadMinidumpFiles(dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot));
        }

        // 意外关机事件
        checkUnexpectedShutdownEvents();

        if (found) {
            emit blueScreenDetected("检测到蓝屏文件，打不良换内存和硬盘");
        }
        QtLogger::WriteLog("DownloadManager: Dump 检测完成");
    });
}

/**
 * @brief Dump 检测（MEMORY.DMP + Minidump + 意外关机事件）
 */
void DownloadManager::uploadMemoryDump(const QString& path)
{
    QFileInfo info(path);
    QString birthTime = info.birthTime().toString("yyyy-MM-dd hh:mm:ss");

    if (isAlreadyUploaded(DUMP_LOG_PATH, birthTime)) return;

    emit dumpFound(path, "1");

    UploadDumpFileInfo dumpInfo;
    dumpInfo.sn = m_sn;
    dumpInfo.mac = m_mac;
    dumpInfo.dataTime = birthTime;
    dumpInfo.uploadTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    dumpInfo.type = "1";
    postDumpMetadata(dumpInfo);
    markAsUploaded(DUMP_LOG_PATH, birthTime);
}

void DownloadManager::uploadMinidumpFiles(const QFileInfoList& files)
{
    for (const QFileInfo& fi : files) {
        QString srcPath = fi.absoluteFilePath();
        QString desktopPath = "C:/Users/Public/Desktop/" + fi.fileName();

        if (QFile::exists(desktopPath)) continue;
        emit dumpFound(srcPath, "2");

        QFile::copy(srcPath, desktopPath);
        QString zipPath = compressDumpFile(desktopPath);
        if (zipPath.isEmpty()) continue;

        std::string url = ConfigManager::instance().buildUrl("uploadFile");
        std::string fileUrl = uploadFile(url, zipPath.toStdString());
        bool success = !fileUrl.empty();

        emit dumpUploaded(QString::fromStdString(fileUrl), success);

        if (success) {
            UploadDumpFileInfo dumpInfo;
            dumpInfo.sn = m_sn;
            dumpInfo.mac = m_mac;
            dumpInfo.dataTime = fi.birthTime().toString("yyyy-MM-dd hh:mm:ss");
            dumpInfo.uploadTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            dumpInfo.fileUrl = QString::fromStdString(fileUrl);
            dumpInfo.type = "1";
            postDumpMetadata(dumpInfo);
        } else {
            QFile::remove(desktopPath);
        }
        QFile::remove(zipPath);
    }
}

QString DownloadManager::compressDumpFile(const QString& sourcePath)
{
    QString zipPath = "C:/Users/Public/Desktop/EDYTest/Tools/BSOD_"
        + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".zip";

    QStringList args;
    args << "a" << "-tzip" << "-r" << "-ssw" << zipPath << sourcePath;

    QProcess process;
    process.setProgram("C:\\Users\\Public\\Desktop\\EDYTest\\Tools\\7za.exe");
    process.setArguments(args);
    process.start();

    if (!process.waitForFinished(300000)) {
        process.kill();
        return "";
    }
    return (process.exitCode() == 0) ? zipPath : "";
}

void DownloadManager::checkUnexpectedShutdownEvents()
{
    QProcess process;
    process.start("powershell", {"-Command",
        "Get-EventLog -LogName System -InstanceId 41 "
        "| Select-Object TimeGenerated, Message "
        "| ConvertTo-Json"});

    if (!process.waitForFinished(10000)) return;

    QJsonDocument doc = QJsonDocument::fromJson(process.readAllStandardOutput());
    if (!doc.isArray()) return;

    for (const QJsonValue& val : doc.array()) {
        QJsonObject obj = val.toObject();
        QString timeStr = obj["TimeGenerated"].toString();

        QString formattedTime;
        if (timeStr.startsWith("/Date(") && timeStr.endsWith(")/")) {
            qint64 msecs = timeStr.mid(6, timeStr.size() - 8).toLongLong();
            QDateTime dt = QDateTime::fromMSecsSinceEpoch(msecs);
            formattedTime = dt.isValid() ? dt.toLocalTime().toString("yyyy-MM-dd HH:mm:ss") : "Invalid";
        } else {
            continue;
        }

        if (isAlreadyUploaded(SHUTDOWN_LOG_PATH, formattedTime)) continue;

        emit dumpFound("", "0");

        UploadDumpFileInfo dumpInfo;
        dumpInfo.sn = m_sn;
        dumpInfo.mac = m_mac;
        dumpInfo.dataTime = formattedTime;
        dumpInfo.uploadTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        dumpInfo.type = "0";

        SerApiModel api;
        if (api.addExtData(dumpInfo.sn.toStdString(), dumpInfo.mac.toStdString(),
                           dumpInfo.fileUrl.toStdString(), dumpInfo.dataTime.toStdString(),
                           dumpInfo.uploadTime.toStdString(), 0)) {
            markAsUploaded(SHUTDOWN_LOG_PATH, formattedTime);
        }
    }
}

void DownloadManager::postDumpMetadata(const UploadDumpFileInfo& info)
{
    SerApiModel api;
    api.addExtData(info.sn.toStdString(), info.mac.toStdString(), info.fileUrl.toStdString(),
                   info.dataTime.toStdString(), info.uploadTime.toStdString(), info.type.toInt());
}

bool DownloadManager::isAlreadyUploaded(const QString& logPath, const QString& key)
{
    if (!QFile::exists(logPath)) return false;
    QFile f(logPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    return QString::fromUtf8(f.readAll()).contains(key);
}

void DownloadManager::markAsUploaded(const QString& logPath, const QString& key)
{
    QFile f(logPath);
    if (!f.open(QIODevice::Append | QIODevice::Text)) return;
    QTextStream out(&f);
    out << key << "\n";
}
