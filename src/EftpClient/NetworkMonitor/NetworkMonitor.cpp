#include "NetworkMonitor.h"
#include "ConfigManager.h"
#include "qtlogger.h"

#include <QProcess>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QtConcurrent>
#include <windows.h>
#include <vector>

/**
 * @brief GBK 解码辅助函数（netsh 在中文 Windows 输出 GBK）
 */
static QString decodeGbk(const QByteArray& raw)
{
    int wideLen = MultiByteToWideChar(936, 0, raw.constData(), raw.size(), nullptr, 0);
    if (wideLen > 0) {
        std::vector<wchar_t> wbuf(wideLen + 1, L'\0');
        MultiByteToWideChar(936, 0, raw.constData(), raw.size(), wbuf.data(), wideLen);
        return QString::fromStdWString(wbuf.data());
    }
    return QString::fromLocal8Bit(raw);
}

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

    // 保存目标 WiFi 信息（监控状态机用）
    m_targetSsid = ssid;
    m_targetPassword = password;
    m_useDefaultWifi = false;

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

/** @brief 启动定时 ping 监控（从配置文件读取间隔和阈值） */
void NetworkMonitor::startMonitor(int intervalMs)
{
    Q_UNUSED(intervalMs);
    auto& cfg = ConfigManager::instance();
    m_pingInterval = cfg.pingIntervalSec() * 1000;
    m_failThreshold = cfg.failThreshold();

    QtLogger::WriteLog(QString("NetworkMonitor: 启动定时监控，间隔 %1 秒，失败阈值 %2 次")
        .arg(cfg.pingIntervalSec()).arg(m_failThreshold));

    // 延迟 2 秒再首次 ping，等待网络就绪
    QTimer::singleShot(2000, this, [this]() {
        checkNetwork();
    });
    m_timer.start(m_pingInterval);
}

void NetworkMonitor::stopMonitor()
{
    m_timer.stop();
    QtLogger::WriteLog("NetworkMonitor: 停止监控");
}

/**
 * @brief 定时检测网络（状态机，3秒一次，累积失败达阈值才切网）
 *
 * 流程：
 *   Normal (3s ping) → 累积 5 次失败 → 切回默认WiFi
 *   → 累积 5 次失败 → 重连目标WiFi → 累积 5 次失败 → 重置网络
 *   任意状态 ping 成功 → 回到 Normal，清零计数
 */
void NetworkMonitor::checkNetwork()
{
    QtConcurrent::run([this]() {
        QString pingTarget = QString::fromStdString(ConfigManager::instance().pingTarget());
        QString currentSsid = getCurrentWifiSsid();

        // 执行 ping
        QProcess process;
        process.start("ping", {"-n", "1", "-w", "3000", pingTarget});
        bool finished = process.waitForFinished(5000);

        int latencyMs = -1;
        bool pingOk = false;

        if (finished) {
            QString output = decodeGbk(process.readAllStandardOutput());
            pingOk = output.contains("TTL");

            QRegularExpression re("(?:时间|time)[<=](\\d+)", QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch match = re.match(output);
            if (match.hasMatch()) {
                latencyMs = match.captured(1).toInt();
            }
        }

        // 发射 ping 结果（UI 更新）
        emit pingResult(currentSsid, latencyMs);

        if (pingOk) {
            // ═══ ping 成功 → 回到正常状态 ═══
            if (!m_isNetworkOk) {
                QtLogger::WriteLog("NetworkMonitor: 网络恢复");
                emit networkRestored();
            }
            m_isNetworkOk = true;
            m_netState = NetState::Normal;
            m_failCount = 0;
            m_resetCount = 0;
        } else {
            // ═══ ping 失败 → 累积计数 ═══
            m_failCount++;
            QtLogger::WriteLog(QString("NetworkMonitor: ping 失败 (第 %1/%2 次), state=%3")
                .arg(m_failCount).arg(m_failThreshold).arg((int)m_netState));

            if (m_isNetworkOk) {
                m_isNetworkOk = false;
                emit networkLost();
            }

            // 累积失败未达阈值，继续 ping
            if (m_failCount < m_failThreshold) {
                return;
            }

            // 累积失败达到阈值 → 状态机转换
            m_failCount = 0;

            switch (m_netState) {
            case NetState::Normal:
                // 累积失败达阈值 → 切回默认 WiFi（不再切回目标）
                m_netState = NetState::FallbackWifi;
                QtLogger::WriteLog("NetworkMonitor: 累积失败达阈值，切回默认 WiFi");
                m_useDefaultWifi = true;
                connectDefaultWifi();
                break;

            case NetState::FallbackWifi:
                // 默认 WiFi 也累积失败 → 重置网络
                m_netState = NetState::ResetNetwork;
                m_resetCount++;
                QtLogger::WriteLog(QString("NetworkMonitor: 默认 WiFi 失败，重置网络 (第 %1 次)").arg(m_resetCount));
                QtConcurrent::run([]() {
                    QProcess::execute("ipconfig", {"/release"});
                    QProcess::execute("ipconfig", {"/renew"});
                });
                break;

            case NetState::ResetNetwork:
                // 重置后仍累积失败 → 回到 FallbackWifi，继续用默认 WiFi
                QtLogger::WriteLog("NetworkMonitor: 重置后仍失败，继续用默认 WiFi 监控");
                m_netState = NetState::FallbackWifi;
                break;
            }
        }
    });
}

/**
 * @brief ping 目标地址检测网络连通性
 */
bool NetworkMonitor::pingBaidu()
{
    QString pingTarget = QString::fromStdString(ConfigManager::instance().pingTarget());
    QProcess process;
    process.start("ping", {"-n", "1", "-w", "3000", pingTarget});
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

    QByteArray raw = process.readAllStandardOutput();
    QString output = decodeGbk(raw);

    QStringList lines = output.split("\n");
    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.startsWith("SSID") && !trimmed.contains("BSSID")) {
            QStringList parts = trimmed.split(":");
            if (parts.size() >= 2) {
                return parts[1].trimmed();
            }
        }
    }
    return "未连接";
}

/**
 * @brief 连接 WiFi（生成 XML 配置文件 + netsh wlan connect，兼容多网卡）
 */
void NetworkMonitor::connectToWifi(const QString& ssid, const QString& password)
{
    QtConcurrent::run([this, ssid, password]() {
        // ===================== 1. 生成 WPA2-PSK 无线网络配置 XML =====================
        QByteArray ssidBytes = ssid.toUtf8();
        QString hexSsid = ssidBytes.toHex().toUpper();

        QString xmlContent = QString(
            "<?xml version=\"1.0\"?>"
            "<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">"
            "    <name>%1</name>"
            "    <SSIDConfig>"
            "        <SSID>"
            "            <hex>%2</hex>"
            "            <name>%1</name>"
            "        </SSID>"
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
            "                <keyMaterial>%3</keyMaterial>"
            "            </sharedKey>"
            "        </security>"
            "    </MSM>"
            "</WLANProfile>"
        ).arg(ssid, hexSsid, password);

        QtLogger::WriteLog("NetworkMonitor: 已生成WiFi配置文件, SSID=" + ssid);

        // 写入临时XML配置文件
        const QString xmlFile = QCoreApplication::applicationDirPath() + "/wifi_profile_temp.xml";
        QFile file(xmlFile);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QtLogger::WriteLog("NetworkMonitor: 错误 - 无法写入临时WiFi配置文件");
            return;
        }
        QTextStream out(&file);
        out << xmlContent;
        file.close();

        // ===================== 2. 向系统添加WiFi配置文件 =====================
        QProcess addProfileProc;
        // 标准参数列表调用，避免命令拼接出错
        addProfileProc.start("netsh", { "wlan", "add", "profile", "filename=" + xmlFile, "user=all" });
        addProfileProc.waitForFinished(3000);
        QString addResult = QString::fromUtf8(addProfileProc.readAllStandardOutput()).trimmed();
        QtLogger::WriteLog("NetworkMonitor: add profile 结果: " + addResult);

        // ===================== 3. 连接WiFi（与原项目方式一致） =====================
        QProcess connectProc;
        connectProc.start("netsh", {"wlan", "connect",
                                     QString("name=%1").arg(ssid),
                                     QString("ssid=%1").arg(ssid)});
        connectProc.waitForFinished(5000);

        if (connectProc.exitCode() == 0) {
            QtLogger::WriteLog("NetworkMonitor: WiFi 连接指令已发送: " + ssid);
            QThread::sleep(2);
            emit wifiConnected(ssid);
        } else {
            QString errMsg = QString::fromUtf8(connectProc.readAllStandardOutput()).trimmed();
            if (errMsg.isEmpty()) errMsg = QString::fromUtf8(connectProc.readAllStandardError()).trimmed();
            QtLogger::WriteLog("NetworkMonitor: WiFi 连接失败: " + errMsg);
        }

        // ===================== 5. 清理临时文件 =====================
        QFile::remove(xmlFile);
        });
}

