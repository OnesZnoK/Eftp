#include "ConfigManager.h"
#include <fstream>
#include <iostream>

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

void ConfigManager::init(const std::string& configsDir)
{
    std::string sep = "/";
    m_app  = loadJsonFile(configsDir + sep + "app.json");
    m_wifi = loadJsonFile(configsDir + sep + "wifi.json");
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

std::string ConfigManager::downloadLocalRoot() const
{
    auto dl = m_app.value("download", json::object());
    return dl.value("localRoot", "C:\\Users\\Public\\Desktop\\EDYTest");
}

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

// ── 原始访问 ──

const json& ConfigManager::appConfig() const
{
    return m_app;
}

const json& ConfigManager::wifiConfig() const
{
    return m_wifi;
}
