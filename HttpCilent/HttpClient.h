#pragma once

#include <string>
#include <map>

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "httplib.h"
#include "../QtLogger/qtlogger.h"

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
    static HttpResult PostRawJson(const std::string& fullUrl,const std::string& jsonPayload,int timeoutSec = 10);

    /**
     * @brief 通用 GET 请求封装
     * @param fullUrl 完整地址 (例如 http://api.example.com/check?sn=123)
     * @param timeoutSec 超时时间（默认10秒）
     * @return 成功返回响应内容
     */
    static HttpResult GetRaw(const std::string& fullUrl,int timeoutSec = 10);

    /**
     * @brief 文件上传
     * @param fullUrl 完整地址 (例如
     * @param filePath 本地文件路径
     * @param fieldName 表单字段名（默认 "file"）
     * @param timeoutSec 超时时间（默认60秒）
     * @return 成功返回服务器响应
     */
    static HttpResult UploadFile(const std::string& fullUrl,
                                 const std::string& filePath,
                                 const std::string& fieldName = "file",
                                 int timeoutSec = 60);

private:
    // 解析 URL，将 http://host:port/path 拆分为 host:port 和 /path
    static bool ParseFullUrl(const std::string& fullUrl,std::string& host,std::string& path);
};
