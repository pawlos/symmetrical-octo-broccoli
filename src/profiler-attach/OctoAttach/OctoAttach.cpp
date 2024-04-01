// OctoAttach.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <sstream>
#include <metahost.h>
#pragma comment(lib, "mscoree.lib")
#include <wchar.h>

int main(int argc, char* argv[])
{
    std::cout << "OctoAttach .NET Framework!\n";
    if (argc <= 1)
    {
        std::cout << "Invalid argument count\n";
        return -1;
    }

    auto pid = std::stoi(argv[1]);

    std::cout << "PID: " << pid << "\n";
    auto handle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
    if (!handle)
    {
        std::cout << "Could not oper process ID: " << pid << std::endl;
        return -2;
    }
    ICLRMetaHost* _metahost = nullptr;
    HRESULT hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID*)&_metahost);
    if (FAILED(hr))
    {
        std::cout << "Could not obtain CLR Mata host.\n";
        return -2;
    }

    IEnumUnknown *enumIterator = nullptr;
    hr = _metahost->EnumerateLoadedRuntimes(handle, &enumIterator);
    if (FAILED(hr))
    {
        std::cout << "Could not obtain Installed runtimes enumerator.\n";
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
        std::cout << "No runtime info loaded found. Only works for .NET Framework applications." << std::endl;
        return -4;
    }

    ICLRProfiling* clrProfiling = nullptr;
    hr = runtimeInfo->GetInterface(CLSID_CLRProfiling, IID_ICLRProfiling, (LPVOID*)&clrProfiling);
    if (FAILED(hr))
    {
        std::cout << "Could not get CLRProfiling interface. " << std::endl;
        return -5;
    }

    static const GUID CLSID_ProfilerCallback3 = { 0x4FD2ED52, 0x7731, 0x4b8d, { 0x94, 0x69, 0x03, 0xD2, 0xCC, 0x30, 0x86, 0xC5 } };

    hr = clrProfiling->AttachProfiler(pid, 2000, &CLSID_ProfilerCallback3, L"c:\\work\\100commitow\\symmetrical-octo-broccoli\\src\\profiler\\x64\\Debug\\OctoProfiler.dll", NULL, 0);
    if (FAILED(hr))
    {
        std::cout << "Could not attach profiler" << std::endl;
        return -6;
    }

    return 0;

}
