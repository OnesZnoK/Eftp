/**
 * @file launcher/main.cpp
 * @brief EFTP 加载器 — 无依赖，读取配置后设置 DLL 搜索路径，启动 EFTP.exe
 *
 * 流程：
 *   1. 检测 exe 目录下的环境标记文件（product/pre/test/...）
 *   2. 读取 configs/dll.ini 获取对应环境的 DLL 路径
 *   3. 调用 AddDllDirectory() 添加 DLL 搜索路径
 *   4. CreateProcess() 启动 EFTP.exe
 */

#include <windows.h>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>

/**
 * @brief 简易 INI 解析器（无外部依赖）
 */
struct IniFile {
    std::map<std::string, std::map<std::string, std::string>> sections;

    bool load(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return false;

        std::string line;
        std::string currentSection;
        while (std::getline(file, line)) {
            // 去掉 \r
            if (!line.empty() && line.back() == '\r') line.pop_back();
            // 跳过空行和注释
            if (line.empty() || line[0] == ';' || line[0] == '#') continue;
            // section
            if (line[0] == '[' && line.back() == ']') {
                currentSection = line.substr(1, line.size() - 2);
                continue;
            }
            // key=value
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                sections[currentSection][key] = value;
            }
        }
        return true;
    }

    std::string get(const std::string& section, const std::string& key, const std::string& defaultVal = "") const {
        auto secIt = sections.find(section);
        if (secIt == sections.end()) return defaultVal;
        auto keyIt = secIt->second.find(key);
        if (keyIt == secIt->second.end()) return defaultVal;
        return keyIt->second;
    }

    std::vector<std::string> sectionNames() const {
        std::vector<std::string> names;
        for (auto& p : sections) names.push_back(p.first);
        return names;
    }
};

/**
 * @brief 检测环境（exe 目录下是否存在同名文件）
 */
std::string detectEnvironment(const std::string& exeDir, const IniFile& ini) {
    for (auto& name : ini.sectionNames()) {
        std::string flagPath = exeDir + "\\" + name;
        DWORD attr = GetFileAttributesA(flagPath.c_str());
        if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
            return name;
        }
    }
    // 默认用第一个 section
    auto names = ini.sectionNames();
    return names.empty() ? "product" : names[0];
}

/**
 * @brief 主函数（读取配置、设置 DLL 路径、启动 EFTP.exe）
 */
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // 获取 exe 所在目录
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string exeDir = exePath;
    exeDir = exeDir.substr(0, exeDir.find_last_of("\\/"));

    // 读取 dll.ini
    IniFile ini;
    std::string iniPath = exeDir + "\\configs\\dll.ini";
    if (!ini.load(iniPath)) {
        MessageBoxA(NULL, "无法读取 configs/dll.ini", "EFTP Launcher", MB_ICONERROR);
        return 1;
    }

    // 检测环境
    std::string env = detectEnvironment(exeDir, ini);

    // 获取 DLL 路径
    std::string corePath = ini.get(env, "core", "dll\\core");
    std::string modulesPath = ini.get(env, "modules", "dll\\modules");

    // 转为绝对路径
    std::string coreAbs = exeDir + "\\" + corePath;
    std::string modulesAbs = exeDir + "\\" + modulesPath;

    // 将 DLL 目录加入 PATH
    char envPath[32768] = {0};
    GetEnvironmentVariableA("PATH", envPath, sizeof(envPath));
    std::string newPath = coreAbs + ";" + modulesAbs + ";" + envPath;
    SetEnvironmentVariableA("PATH", newPath.c_str());

    // 构建命令行：EFTP.exe + 原始参数
    std::string cmdLine = "\"" + exeDir + "\\EFTP.exe\"";
    LPSTR cmdLineRaw = GetCommandLineA();
    // 跳过 launcher 自身的路径，传递剩余参数
    std::string fullCmd(cmdLineRaw);
    // 找到第一个空格后的内容作为参数
    size_t argStart = fullCmd.find(' ');
    if (argStart != std::string::npos) {
        cmdLine += " " + fullCmd.substr(argStart + 1);
    }

    // 以管理员身份启动 EFTP.exe，最大化窗口
    std::string eftpPath = exeDir + "\\EFTP.exe";
    std::string params;
    if (argStart != std::string::npos) {
        params = fullCmd.substr(argStart + 1);
    }

    SHELLEXECUTEINFOA sei = { sizeof(sei) };
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = "runas";           // 以管理员身份运行
    sei.lpFile = eftpPath.c_str();
    sei.lpParameters = params.empty() ? NULL : params.c_str();
    sei.nShow = SW_SHOWMAXIMIZED;   // 最大化窗口

    if (!ShellExecuteExA(&sei)) {
        char msg[256];
        sprintf_s(msg, "无法以管理员身份启动 EFTP.exe (错误码: %lu)", GetLastError());
        MessageBoxA(NULL, msg, "EFTP Launcher", MB_ICONERROR);
        return 1;
    }

    // 等待子进程结束
    WaitForSingleObject(sei.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(sei.hProcess, &exitCode);

    CloseHandle(sei.hProcess);

    return (int)exitCode;
}
