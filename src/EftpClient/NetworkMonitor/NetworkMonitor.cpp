#include "NetworkMonitor.h"
#include "ConfigManager.h"
#include "qtlogger.h"

#include <QProcess>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QtConcurrent>

NetworkMonitor::NetworkMonitor(QObject* parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &NetworkMonitor::checkNetwork);
}

NetworkMonitor::~NetworkMonitor()
{
    stopMonitor();
}

/**
 * @brief 连接默认 WiFi（程序启动时调用）
 */
void NetworkMonitor::connectDefaultWifi()
{
    auto& cfg = ConfigManager::instance();
    auto wifi = cfg.defaultWifi();
    QString ssid = QString::fromStdString(wifi.first);
    QString password = QString::fromStdString(wifi.second);

    if (ssid.isEmpty()) {
        QtLogger::WriteLog("NetworkMonitor: 未配置默认 WiFi");
        return;
    }

    QtLogger::WriteLog("NetworkMonitor: 连接默认 WiFi: " + ssid);
    connectToWifi(ssid, password);
}

/**
 * @brief 根据工序连接对应 WiFi（获取到工序信息后调用）
 * @param routeId 工序 ID，用于从 wifi.json 查找目标 SSID
 */
void NetworkMonitor::connectForRoute(int routeId)
{
    auto& cfg = ConfigManager::instance();
    auto wifi = cfg.wifiForRoute(routeId);
    QString ssid = QString::fromStdString(wifi.first);
    QString password = QString::fromStdString(wifi.second);

    if (ssid.isEmpty()) {
        QtLogger::WriteLog(QString("NetworkMonitor: 工序 %1 无对应 WiFi 配置").arg(routeId));
        return;
    }

    // 检查当前是否已在目标 WiFi
    QString currentSsid = getCurrentWifiSsid();
    if (currentSsid == ssid) {
        QtLogger::WriteLog("NetworkMonitor: 已连接目标 WiFi: " + ssid);
        emit wifiConnected(ssid);
        return;
    }

    QtLogger::WriteLog(QString("NetworkMonitor: 切换 WiFi: %1 → %2").arg(currentSsid, ssid));
    emit wifiSwitching(ssid);
    connectToWifi(ssid, password);
}

/** @brief 启动定时 ping 监控 */
void NetworkMonitor::startMonitor(int intervalMs)
{
    QtLogger::WriteLog(QString("NetworkMonitor: 启动定时监控，间隔 %1 秒").arg(intervalMs / 1000));
    checkNetwork();      // 立即执行一次
    m_timer.start(intervalMs);
}

void NetworkMonitor::stopMonitor()
{
    m_timer.stop();
    QtLogger::WriteLog("NetworkMonitor: 停止监控");
}

/**
 * @brief 定时检测网络（ping + WiFi 状态检查）
 */
void NetworkMonitor::checkNetwork()
{
    QtConcurrent::run([this]() {
        // 从配置读取 ping 目标
        QString pingTarget = QString::fromStdString(ConfigManager::instance().pingTarget());
        QString currentSsid = getCurrentWifiSsid();

        // 执行 ping 并获取延迟
        QProcess process;
        process.start("ping", {"-n", "1", "-w", "3000", pingTarget});
        bool finished = process.waitForFinished(5000);

        int latencyMs = -1;
        bool pingOk = false;

        if (finished) {
            QString output = process.readAllStandardOutput();
            pingOk = output.contains("TTL");

            // 提取延迟：中文系统 "时间=XXms" 或英文 "time=XXms"
            QRegularExpression re("(?:时间|time)[<=](\\d+)", QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch match = re.match(output);
            if (match.hasMatch()) {
                latencyMs = match.captured(1).toInt();
            }
        }

        // 发射 ping 结果（UI 更新）
        emit pingResult(currentSsid, latencyMs);

        if (pingOk) {
            if (!m_isNetworkOk) {
                QtLogger::WriteLog("NetworkMonitor: 网络恢复");
                emit networkRestored();
            }
            m_isNetworkOk = true;
            m_failCount = 0;
        } else {
            m_failCount++;
            QtLogger::WriteLog(QString("NetworkMonitor: ping 失败 (第 %1 次)").arg(m_failCount));

            if (m_isNetworkOk) {
                m_isNetworkOk = false;
                emit networkLost();
            }

            if (m_failCount >= 3) {
                reconnectNetwork();
            }
        }
    });
}

/**
 * @brief ping 目标地址检测网络连通性
 */
bool NetworkMonitor::pingBaidu()
{
    QProcess process;
    process.start("ping", {"-n", "1", "-w", "3000", "www.baidu.com"});
    if (!process.waitForFinished(5000)) {
        return false;
    }
    QString output = process.readAllStandardOutput();
    return output.contains("TTL");
}

/**
 * @brief 获取当前 WiFi SSID（通过 netsh wlan show interfaces）
 */
QString NetworkMonitor::getCurrentWifiSsid()
{
    QProcess process;
    process.start("netsh", {"wlan", "show", "interfaces"});
    if (!process.waitForFinished(5000)) {
        return "未知";
    }

    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    QStringList lines = output.split("\r\n");
    for (const QString& line : lines) {
        if (line.trimmed().startsWith("SSID") && !line.contains("BSSID")) {
            QStringList parts = line.split(":");
            if (parts.size() >= 2) {
                return parts[1].trimmed();
            }
        }
    }
    return "未连接";
}

/**
 * @brief 连接 WiFi（生成 XML 配置文件 + netsh wlan connect）
 */
void NetworkMonitor::connectToWifi(const QString& ssid, const QString& password)
{
    QtConcurrent::run([ssid, password]() {
        // 生成 WLAN 配置文件
        QString xmlContent = QString(
            "<?xml version=\"1.0\"?>"
            "<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">"
            "    <name>%1</name>"
            "    <SSIDConfig>"
            "        <SSID><name>%1</name></SSID>"
            "    </SSIDConfig>"
            "    <connectionType>ESS</connectionType>"
            "    <connectionMode>auto</connectionMode>"
            "    <MSM>"
            "        <security>"
            "            <authEncryption>"
            "                <authentication>WPA2PSK</authentication>"
            "                <encryption>AES</encryption>"
            "                <useOneX>false</useOneX>"
            "            </authEncryption>"
            "            <sharedKey>"
            "                <keyType>passPhrase</keyType>"
            "                <protected>false</protected>"
            "                <keyMaterial>%2</keyMaterial>"
            "            </sharedKey>"
            "        </security>"
            "    </MSM>"
            "</WLANProfile>"
        ).arg(ssid, password);

        QString xmlFile = QCoreApplication::applicationDirPath() + "/wifi_profile.xml";
        QFile file(xmlFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << xmlContent;
            file.close();
        } else {
            QtLogger::WriteLog("NetworkMonitor: 无法写入 WiFi 配置文件");
            return;
        }

        QProcess addProfile;
        addProfile.start("netsh", {"wlan", "add", "profile",
                                    QString("filename=%1").arg(xmlFile), "user=all"});
        addProfile.waitForFinished();

        QProcess connect;
        connect.start("netsh", {"wlan", "connect",
                                 QString("name=%1").arg(ssid),
                                 QString("ssid=%1").arg(ssid)});
        connect.waitForFinished();

        if (connect.exitCode() == 0) {
            QtLogger::WriteLog("NetworkMonitor: WiFi 连接指令已发送: " + ssid);
        } else {
            QtLogger::WriteLog("NetworkMonitor: WiFi 连接失败: " +
                QString::fromLocal8Bit(connect.readAllStandardError()));
        }
    });
}

/**
 * @brief 网络重连（ipconfig /release + /renew）
 */
void NetworkMonitor::reconnectNetwork()
{
    QtLogger::WriteLog("NetworkMonitor: 尝试网络重连 (ipconfig /renew)");
    QtConcurrent::run([]() {
        QProcess::execute("ipconfig", {"/release"});
        QProcess::execute("ipconfig", {"/renew"});
    });
}
