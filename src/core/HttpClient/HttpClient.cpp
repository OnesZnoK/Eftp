#include "HttpClient.h"
#include <regex>
#include <QEventLoop>
#include <QTimer>

// 重试参数
constexpr int DEFAULT_TIMEOUT_SEC = 10;
constexpr int RETRY_INTERVAL_MS = 3000;  // 所有请求统一 3 秒重试

bool HttpClient::ParseFullUrl(const std::string& fullUrl, std::string& host, std::string& path)
{
    std::regex urlRegex(R"(^(http|https)://([^/]+)(/.*)?$)");
    std::smatch match;
    if (std::regex_search(fullUrl, match, urlRegex))
    {
        host = match[1].str() + "://" + match[2].str();
        path = match[3].str();
        if (path.empty()) path = "/";
        return true;
    }
    return false;
}

/**
 * @brief POST 请求（无限重试，2秒间隔）
 *        服务端正常响应（有 code 字段）不重试
 */
HttpResult HttpClient::PostRawJson(const std::string& fullUrl, const std::string& jsonPayload, int timeoutSec)
{
    HttpResult result;
    std::string host, path;
    if (!ParseFullUrl(fullUrl, host, path))
    {
        QtLogger::WriteLog("URL解析失败: " + QString::fromStdString(fullUrl), enLogType::WARNING);
        return result;
    }

    int retryCount = 0;
    int timeout = timeoutSec > 0 ? timeoutSec : DEFAULT_TIMEOUT_SEC;

    while (true)
    {
        retryCount++;
        httplib::Client cli(host);
        cli.set_connection_timeout(timeout);
        cli.set_read_timeout(timeout);

        auto res = cli.Post(path, jsonPayload, "application/json");

        if (res) {
            result.status = res->status;
            result.body = res->body;
            result.success = (res->status >= 200 && res->status < 300);

            if (result.success) {
                if (retryCount > 1) {
                    QtLogger::WriteLog(QString("POST成功 [%1] 代码:%2 (重试:%3)")
                        .arg(path.c_str()).arg(res->status).arg(retryCount - 1));
                }
                return result;
            }

            // 服务端正常响应（有 code 字段），不重试
            if (result.body.find("\"code\"") != std::string::npos) {
                QtLogger::WriteLog(QString("POST服务端报错 [%1] 代码:%2").arg(path.c_str()).arg(res->status));
                return result;
            }
        }

        QtLogger::WriteLog(QString("POST失败，3秒后重试 (第 %1 次)").arg(retryCount));
        QEventLoop waitLoop;
        QTimer::singleShot(RETRY_INTERVAL_MS, &waitLoop, &QEventLoop::quit);
        waitLoop.exec();
    }
}

/**
 * @brief GET 请求（无限重试，每 3 秒重试）
 */
HttpResult HttpClient::GetRaw(const std::string& fullUrl, int timeoutSec)
{
    HttpResult result;
    std::string host, path;

    QtLogger::WriteLog(QString("发起GET请求: %1").arg(QString::fromStdString(fullUrl)));

    if (!ParseFullUrl(fullUrl, host, path)) {
        QtLogger::WriteLog("URL解析失败: " + QString::fromStdString(fullUrl));
        return result;
    }

    int retryCount = 0;
    int timeout = timeoutSec > 0 ? timeoutSec : DEFAULT_TIMEOUT_SEC;

    while (true)
    {
        retryCount++;
        httplib::Client cli(host);
        cli.set_connection_timeout(timeout);
        cli.set_read_timeout(timeout);

        auto res = cli.Get(path);

        if (res && res.error() == httplib::Error::Success) {
            result.status = res->status;
            result.body = res->body;
            result.success = (res->status == 200);

            if (result.success) {
                if (retryCount > 1) {
                    QtLogger::WriteLog(QString("GET请求成功，状态码: %1 (重试:%2)")
                        .arg(result.status).arg(retryCount - 1));
                }
                return result;
            }
        }

        QtLogger::WriteLog(QString("GET失败，3秒后重试 (第 %1 次)").arg(retryCount));
        QEventLoop waitLoop;
        QTimer::singleShot(RETRY_INTERVAL_MS, &waitLoop, &QEventLoop::quit);
        waitLoop.exec();
    }
}
