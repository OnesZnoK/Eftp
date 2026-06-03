#pragma once

/**
 * @file ConfigManager.h
 * @brief 配置管理模块（单例）
 *
 * 功能：
 *   读取 configs/app.json 和 configs/wifi.json，提供全局配置访问。
 *   所有模块通过 ConfigManager::instance() 获取配置，不直接读文件。
 *
 * 配置文件：
 *   - app.json  — 服务端地址、接口路径、下载参数、网络配置、日志级别
 *   - wifi.json — 默认WiFi、工序→WiFi路由映射
 *
 * 使用方式：
 * @code
 *   // main.cpp 启动时初始化一次
 *   ConfigManager::instance().init(exeDir + "/configs");
 *
 *   // ServerApi 中拼接口地址
 *   std::string url = ConfigManager::instance().buildUrl("queryDeviceInfo");
 *   // → "http://eftp.edianyun.com/api/client/queryDeviceInfo"
 *
 *   // NetworkMonitor 中取 ping 目标
 *   std::string target = ConfigManager::instance().pingTarget();
 *
 *   // DownloadManager 中取下载参数
 *   int retry = ConfigManager::instance().maxRetry();
 *   std::string tool = ConfigManager::instance().tool7zaPath();
 *
 *   // NetworkMonitor 中取工序对应WiFi
 *   auto wifi = ConfigManager::instance().wifiForRoute(526);
 *   // → {"e-shangxiajia", "edz123456"}
 * @endcode
 *
 * 依赖模块：JsonHpp（JSON解析）
 * 被依赖方：SerApiModel、DownloadManager、NetworkMonitor、DumpUploader
 */

#include <string>
#include <map>
#include <utility>

#include "json.hpp"

#ifdef CONFIGMANAGER_LIBRARY
#  define CONFIGMANAGER_EXPORT __declspec(dllexport)
#else
#  define CONFIGMANAGER_EXPORT __declspec(dllimport)
#endif

using json = nlohmann::json;

/**
 * @brief 配置管理器（单例）
 *        读取 app.json + wifi.json，提供全局配置访问
 */
class CONFIGMANAGER_EXPORT ConfigManager
{
public:
    static ConfigManager& instance();

    /**
     * @brief 加载配置文件
     * @param configsDir configs 目录的绝对路径
     */
    void init(const std::string& configsDir);

    // ── 环境 ──
    std::string environment() const;
    std::string activeServerId() const;   // 当前服务器ID

    // ── 接口地址 ──
    std::string baseUrl() const;
    std::string buildUrl(const std::string& apiKey) const;

    // ── 下载参数 ──
    std::string downloadLocalRoot() const;
    std::string tool7zaPath() const;
    int maxRetry() const;
    int downloadTimeoutMs() const;
    int extractTimeoutMs() const;

    // ── 网络 ──
    std::string pingTarget() const;
    int pingIntervalSec() const;

    // ── WiFi ──
    std::pair<std::string, std::string> defaultWifi() const;
    std::pair<std::string, std::string> wifiForRoute(int routeId) const;
    bool hasWifiRoute(int routeId) const;

    // ── 日志 ──
    std::string logLevel() const;

    // ── 原始访问（备用） ──
    const json& appConfig() const;
    const json& wifiConfig() const;

private:
    ConfigManager();
    ~ConfigManager();
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    json loadJsonFile(const std::string& filePath) const;

    json m_app;
    json m_wifi;
    bool m_initialized = false;
};
