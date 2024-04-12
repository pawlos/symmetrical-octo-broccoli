// OctoAttach.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <sstream>
#include <format>
#include <metahost.h>
#pragma comment(lib, "mscoree.lib")
#include <wchar.h>

void Error(const std::string& _s)
{
    std::cerr << "\033[31m" << _s << "\033[0m";
    std::cerr << std::endl;
    std::cerr.flush();
}

int main(int argc, char* argv[])
{
    std::cout << "OctoAttach .NET Framework!\n";
    int pid = 0;
    if (argc <= 1)
    {
        std::cout << "Provide pid: ";
        std::cin >> pid;
    }
    else
    {
        pid = std::stoi(argv[1]);
    }

    std::cout << "PID: " << pid << "\n";
    auto handle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
    if (!handle)
    {
        Error(std::format("Could not oper process ID: {}", pid));
        return -2;
    }
    ICLRMetaHost* _metahost = nullptr;
    HRESULT hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID*)&_metahost);
    if (FAILED(hr))
    {
        Error("Could not obtain CLR Mata host");
        return -2;
    }

    IEnumUnknown *enumIterator = nullptr;
    hr = _metahost->EnumerateLoadedRuntimes(handle, &enumIterator);
    if (FAILED(hr))
    {
        Error("Could not obtain Installed runtimes enumerator");
        return -3;
    }
    ICLRRuntimeInfo *runtimeInfo = nullptr;
    ULONG retreivedNum{};
    hr = enumIterator->Next(1, reinterpret_cast<IUnknown**>(&runtimeInfo), &retreivedNum);
    while (retreivedNum > 0)
    {
        WCHAR runtimeVersion[255];
        ULONG versionInfoLen;
        hr = runtimeInfo->GetVersionString(runtimeVersion, &versionInfoLen);
        auto _version = std::wstring(runtimeVersion);
        const std::string version(_version.begin(), _version.end());
        std::cout << "Loaded runtime: " << version  << std::endl;
        if (runtimeInfo)
        {
            break;
        }
        hr = enumIterator->Next(1, reinterpret_cast<IUnknown**>(&runtimeInfo), &retreivedNum);
    }

    if (!runtimeInfo)
    {
        Error("No runtime info loaded found. Only works for .NET Framework applications");
        return -4;
    }

    ICLRProfiling* clrProfiling = nullptr;
    hr = runtimeInfo->GetInterface(CLSID_CLRProfiling, IID_ICLRProfiling, (LPVOID*)&clrProfiling);
    if (FAILED(hr))
    {
        Error("Could not get CLRProfiling interface");
        return -5;
    }

    static const GUID CLSID_ProfilerCallback3 = { 0x4FD2ED52, 0x7731, 0x4b8d, { 0x94, 0x69, 0x03, 0xD2, 0xCC, 0x30, 0x86, 0xC5 } };

    hr = clrProfiling->AttachProfiler(pid, 2000, &CLSID_ProfilerCallback3, L"c:\\work\\100commitow\\symmetrical-octo-broccoli\\src\\profiler\\x64\\Debug\\OctoProfiler.dll", NULL, 0);
    if (FAILED(hr))
    {
        Error(std::format("Could not attach profiler. Hr = {0:X}", hr));
        return -6;
    }
    WaitForSingleObject(handle, INFINITE);
    return 0;

}
