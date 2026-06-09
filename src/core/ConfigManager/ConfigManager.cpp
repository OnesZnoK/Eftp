#include "ConfigManager.h"
#include <fstream>
#include <iostream>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>

ConfigManager::ConfigManager()
{
}

ConfigManager::~ConfigManager()
{
}

ConfigManager& ConfigManager::instance()
{
    static ConfigManager inst;
    return inst;
}

json ConfigManager::loadJsonFile(const std::string& filePath) const
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[ConfigManager] cannot open: " << filePath << std::endl;
        return json();
    }
    try {
        return json::parse(file);
    } catch (const json::parse_error& e) {
        std::cerr << "[ConfigManager] parse error: " << e.what() << std::endl;
        return json();
    }
}

/**
 * @brief 深度合并：将 overlay 的值覆盖到 base 上（递归合并对象）
 */
static void deepMerge(json& base, const json& overlay)
{
    if (!overlay.is_object() || !base.is_object()) {
        base = overlay;
        return;
    }
    for (auto it = overlay.begin(); it != overlay.end(); ++it) {
        if (it.value().is_object() && base.contains(it.key()) && base[it.key()].is_object()) {
            deepMerge(base[it.key()], it.value());
        } else {
            base[it.key()] = it.value();
        }
    }
}

void ConfigManager::init(const std::string& configsDir)
{
    std::string sep = "/";
    std::string env = detectEnvironment();

    // 加载基础配置
    m_app  = loadJsonFile(configsDir + sep + "app.json");
    m_wifi = loadJsonFile(configsDir + sep + "wifi.json");

    // 加载环境文件夹下的覆盖配置（configs/{env}/app.json）
    std::string envDir = configsDir + sep + env;
    json envApp  = loadJsonFile(envDir + sep + "app.json");
    json envWifi = loadJsonFile(envDir + sep + "wifi.json");

    if (!envApp.is_null()  && !envApp.empty())  deepMerge(m_app,  envApp);
    if (!envWifi.is_null() && !envWifi.empty()) deepMerge(m_wifi, envWifi);

    m_initialized = true;
}

// ── 环境 ──

std::string ConfigManager::environment() const
{
    return m_app.value("environment", "product");
}

std::string ConfigManager::activeServerId() const
{
    return "eftp";
}

// ── 接口地址 ──

std::string ConfigManager::baseUrl() const
{
    std::string sid = activeServerId();
    auto servers = m_app.value("servers", json::object());
    auto server  = servers.value(sid, json::object());
    return server.value("baseUrl", "");
}

std::string ConfigManager::buildUrl(const std::string& apiKey) const
{
    std::string sid = activeServerId();
    auto servers = m_app.value("servers", json::object());
    auto server  = servers.value(sid, json::object());
    auto apis    = server.value("apis", json::object());
    std::string path = apis.value(apiKey, "");
    return server.value("baseUrl", "") + path;
}

// ── 下载参数 ──

std::string ConfigManager::tool7zaPath() const
{
    auto dl = m_app.value("download", json::object());
    return dl.value("tool7za", "C:\\Users\\Public\\Desktop\\EDYTest\\Tools\\7za.exe");
}

int ConfigManager::maxRetry() const
{
    auto dl = m_app.value("download", json::object());
    return dl.value("maxRetry", 30);
}

int ConfigManager::downloadTimeoutMs() const
{
    auto dl = m_app.value("download", json::object());
    return dl.value("timeoutMs", 900000);
}

int ConfigManager::extractTimeoutMs() const
{
    auto dl = m_app.value("download", json::object());
    return dl.value("extractTimeoutMs", 30000);
}

// ── 网络 ──

std::string ConfigManager::pingTarget() const
{
    auto net = m_app.value("network", json::object());
    return net.value("pingTarget", "www.baidu.com");
}

int ConfigManager::pingIntervalSec() const
{
    auto net = m_app.value("network", json::object());
    return net.value("pingIntervalSec", 8);
}

int ConfigManager::latencyThresholdMs() const
{
    auto net = m_app.value("network", json::object());
    return net.value("latencyThresholdMs", 800);
}

// ── WiFi ──

std::pair<std::string, std::string> ConfigManager::defaultWifi() const
{
    auto def = m_wifi.value("default", json::object());
    return { def.value("ssid", ""), def.value("password", "") };
}

std::pair<std::string, std::string> ConfigManager::wifiForRoute(int routeId) const
{
    auto mapping = m_wifi.value("routeMapping", json::object());
    std::string key = std::to_string(routeId);
    if (mapping.contains(key)) {
        auto entry = mapping[key];
        return { entry.value("ssid", ""), entry.value("password", "") };
    }
    return defaultWifi();
}

bool ConfigManager::hasWifiRoute(int routeId) const
{
    auto mapping = m_wifi.value("routeMapping", json::object());
    return mapping.contains(std::to_string(routeId));
}

// ── 日志 ──

std::string ConfigManager::logLevel() const
{
    auto log = m_app.value("log", json::object());
    return log.value("level", "debug");
}

// ── 环境检测 ──

std::string ConfigManager::detectEnvironment() const
{
    // 从 dll.ini 读取所有 section 名
    // 检查 exe 目录下是否存在同名文件，第一个匹配的即为当前环境
    QString exeDir = QCoreApplication::applicationDirPath();
    QString iniPath = exeDir + "/configs/dll.ini";
    QSettings settings(iniPath, QSettings::IniFormat);

    for (const QString& group : settings.childGroups()) {
        if (QFile::exists(exeDir + "/" + group)) {
            return group.toStdString();
        }
    }

    // 都没找到，用第一个 section 作为默认
    QStringList groups = settings.childGroups();
    return groups.isEmpty() ? "product" : groups.first().toStdString();
}

// ── DLL 路径（从 dll.ini 按环境读取）──

std::string ConfigManager::dllPathCore() const
{
    QString iniPath = QCoreApplication::applicationDirPath() + "/configs/dll.ini";
    QSettings settings(iniPath, QSettings::IniFormat);
    QString env = QString::fromStdString(detectEnvironment());
    settings.beginGroup(env);
    std::string path = settings.value("core", "dll/core").toString().toStdString();
    settings.endGroup();
    return path;
}

std::string ConfigManager::dllPathModules() const
{
    QString iniPath = QCoreApplication::applicationDirPath() + "/configs/dll.ini";
    QSettings settings(iniPath, QSettings::IniFormat);
    QString env = QString::fromStdString(detectEnvironment());
    settings.beginGroup(env);
    std::string path = settings.value("modules", "dll/modules").toString().toStdString();
    settings.endGroup();
    return path;
}

// ── 原始访问 ──

const json& ConfigManager::appConfig() const
{
    return m_app;
}

const json& ConfigManager::wifiConfig() const
{
    return m_wifi;
}
