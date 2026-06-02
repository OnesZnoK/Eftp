#include "HttpClient.h"
#include <regex>


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

HttpResult HttpClient::PostRawJson(const std::string& fullUrl, const std::string& jsonPayload, int timeoutSec)
{
    HttpResult result;
    std::string host, path;
    if (!ParseFullUrl(fullUrl, host, path))
    {
        QtLogger::WriteLog("URL解析失败: " + QString::fromStdString(fullUrl), enLogType::WARNING);
        return result;
    }

    httplib::Client cli(host);
    cli.set_connection_timeout(timeoutSec);
    cli.set_read_timeout(timeoutSec);

    auto res = cli.Post(path, jsonPayload, "application/json");

    if (res) {
        result.status = res->status;
        result.body = res->body;
        result.success = (res->status >= 200 && res->status < 300);

        if (result.success) {
            QtLogger::WriteLog(QString("POST成功 [%1] 代码:%2").arg(path.c_str()).arg(res->status));
        }
        else {
            QtLogger::WriteLog(QString("POST服务端报错 [%1] 代码:%2").arg(path.c_str()).arg(res->status), enLogType::WARNING);
        }
    }
    else {
        QtLogger::WriteLog(QString("POST连接失败 (无法触达服务器) [%1]").arg(host.c_str()), enLogType::SERIOUS);
    }

    return result;
}

HttpResult HttpClient::GetRaw(const std::string& fullUrl, int timeoutSec)
{
    int count = 0;
	const int maxAttempts = 3;
    HttpResult result;
    std::string host, path;

    QtLogger::WriteLog(QString("发起GET请求: %1").arg(QString::fromStdString(fullUrl)));

    if (!ParseFullUrl(fullUrl, host, path)) {
        QtLogger::WriteLog("URL解析失败: " + QString::fromStdString(fullUrl));
        return result;
    }

loop:
    count++;
    httplib::Client cli(host);
    cli.set_connection_timeout(timeoutSec);
    cli.set_read_timeout(timeoutSec);

    auto res = cli.Get(path);

    if (res && res.error() == httplib::Error::Success) {
        result.status = res->status;
        result.body = res->body;
        result.success = (res->status == 200);
    }
    else {
        int errCode = res ? (int)res.error() : -1;
        result.success = false;
        result.status = -1;
        QtLogger::WriteLog(QString("第 %1 次请求底层失败，错误码: %2").arg(count).arg(errCode));
    }

    if (result.status == -1 && count < maxAttempts)
    {
        QtLogger::WriteLog(QString("请求失败，正在准备第 %1 次重试...").arg(count));
        goto loop;
    }

    if (!result.success) {
        QtLogger::WriteLog(QString("GET请求最终失败，尝试次数: %1").arg(count));
    }
    else {
        QtLogger::WriteLog(QString("GET请求成功，状态码: %1").arg(result.status));
    }

    return result;
}