#pragma once

/**
 * @file HttpClient.h
 * @brief HTTP 客户端库（SHARED DLL）
 *
 * 功能：
 *   - GET/POST 请求（无限重试，每 5 秒重试一次）
 *   - 超时控制（默认 10 秒）
 *
 * 业务逻辑：
 *   1. 解析 URL 拆分 host + path
 *   2. 发起 HTTP 请求
 *   3. 服务端正常响应（有 code 字段）→ 返回结果
 *   4. 网络失败 → 每 5 秒重试，直到成功
 */

#include <string>
#include <map>
#include <functional>

#include "httplib.h"
#include "qtlogger.h"

#ifdef HTTPCLIENT_LIBRARY
#  define HTTPCLIENT_EXPORT Q_DECL_EXPORT
#else
#  define HTTPCLIENT_EXPORT Q_DECL_IMPORT
#endif

/**
 * @brief HTTP 请求结果封装
 * @param status HTTP 状态码
 * @param success 请求是否成功
 * @param body 响应正文内容
 */
struct HTTPCLIENT_EXPORT HttpResult {
    int status = -1;
    bool success = false;
    std::string body;
};

/**
 * @brief 重试回调函数类型
 * @param retryCount 当前重试次数
 * @param url 请求的 URL
 */
using RetryCallback = std::function<void(int retryCount, const std::string& url)>;

class HTTPCLIENT_EXPORT HttpClient
{
public:
    /**
     * @brief 通用 POST 请求封装 (JSON 格式)
     * @param fullUrl 完整地址 (例如 http://192.168.1.100:8080/api/report)
     * @param jsonPayload 请求体内容
     * @param timeoutSec 超时时间（默认10秒）
     * @return 成功返回服务器响应，失败返回空
     */
    static HttpResult PostRawJson(const std::string& fullUrl,
                                  const std::string& jsonPayload,
                                  int timeoutSec = 10);

    /**
     * @brief 通用 GET 请求封装
     * @param fullUrl 完整地址 (例如 http://api.example.com/check?sn=123)
     * @param timeoutSec 超时时间（默认10秒）
     * @param onRetry 重试回调（每次重试时调用，用于 UI 提示）
     * @return 成功返回响应内容
     */
    static HttpResult GetRaw(const std::string& fullUrl,
                             int timeoutSec = 10,
                             RetryCallback onRetry = nullptr);

private:
    // 解析 URL，将 http://host:port/path 拆分为 host:port 和 /path
    static bool ParseFullUrl(const std::string& fullUrl,
                             std::string& host,
                             std::string& path);
};
