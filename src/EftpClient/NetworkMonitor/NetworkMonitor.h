#pragma once

/**
 * @file NetworkMonitor.h
 * @brief 网络监控模块（SHARED DLL）
 *
 * 功能：
 *   - 定时 ping 检测网络连通性
 *   - 检测当前 WiFi SSID
 *   - 根据工序自动切换 WiFi
 *   - 网络异常时自适应重连
 *
 * 业务逻辑：
 *   1. 程序启动 → 连接默认 WiFi（wifi.json 的 default 配置）
 *   2. 查询到工序信息 → 根据 routeId 从 wifi.json 的 routeMapping 查找目标 WiFi
 *   3. 当前 WiFi 与目标不一致 → 生成 XML 配置文件 → netsh wlan connect 切换
 *   4. 定时 ping 百度检测连通性，失败时自动重连
 *   5. 延迟超过阈值（配置文件 latencyThresholdMs）→ 提示网络环境差
 *
 * 配置来源：wifi.json + app.json（通过 ConfigManager 读取）
 */
#include <QObject>
#include <QTimer>
#include <QString>

#include <QtCore/qglobal.h>

#ifdef NETWORKMONITOR_LIBRARY
#  define NETWORKMONITOR_EXPORT Q_DECL_EXPORT
#else
#  define NETWORKMONITOR_EXPORT Q_DECL_IMPORT
#endif

class NETWORKMONITOR_EXPORT NetworkMonitor : public QObject
{
    Q_OBJECT
public:
    explicit NetworkMonitor(QObject* parent = nullptr);
    ~NetworkMonitor();

    /**
     * @brief 连接默认 WiFi（程序启动时调用）
     */
    void connectDefaultWifi();

    /**
     * @brief 根据工序连接对应 WiFi（获取到工序信息后调用）
     * @param routeId 工序 ID，用于从 wifi.json 查找目标 SSID
     */
    void connectForRoute(int routeId);

    /**
     * @brief 启动定时 ping 监控
     * @param intervalMs 检查间隔（毫秒），默认 8 秒
     */
    void startMonitor(int intervalMs = 5000);

    /**
     * @brief 停止监控
     */
    void stopMonitor();

signals:
    void networkLost();
    void networkRestored();
    void wifiSwitching(const QString& ssid);
    void wifiConnected(const QString& ssid);
    void pingResult(const QString& ssid, int latencyMs);  // WiFi名 + ping延迟（-1=超时）

private slots:
    void checkNetwork();

private:
    bool pingBaidu();
    QString getCurrentWifiSsid();
    void connectToWifi(const QString& ssid, const QString& password);

    /** @brief 网络监控状态 */
    enum class NetState {
        Normal,          // 正常：3s ping
        FallbackWifi,    // 累积失败达阈值：切回默认WiFi（不再切回目标）
        ResetNetwork     // 默认WiFi也失败：重置网络
    };

    QTimer m_timer;
    bool m_isNetworkOk = true;
    int m_failCount = 0;            // 累积失败次数
    NetState m_netState = NetState::Normal;
    bool m_useDefaultWifi = false;  // true=已切回默认WiFi，不再切回目标
    int m_resetCount = 0;           // 重置网络次数
    QString m_targetSsid;           // 目标 WiFi SSID（工序对应）
    QString m_targetPassword;       // 目标 WiFi 密码

    int m_pingInterval = 3000;   // ping 间隔（从配置读取）
    int m_failThreshold = 5;     // 累积失败阈值（从配置读取）
    static constexpr int RESET_INTERVAL = 10000;    // 重置网络后等待
};
