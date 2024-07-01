#include <iostream>
#include <string>
#include <sstream>
#include <format>
#include <metahost.h>
#pragma comment(lib, "mscoree.lib")
#include <Shlwapi.h>

void Error(const std::string& _s)
{
    std::cerr << "\033[31m" << _s << "\033[0m";
    std::cerr << '\n';
    std::cerr.flush();
}

void Error(const std::wstring& _s)
{
    const std::string s(_s.begin(), _s.end());
    Error(s);
}

constexpr auto ErrorCouldNotOpenProcess = -1;
constexpr auto CLRErrorNoMetaHost = -2;
constexpr auto CLRErrorNoInstalledRuntimesFound = -3;
constexpr auto CLRErrorNoLoadedRuntimesFound = -4;
constexpr auto CLRErrorNoProfilingInterface = -5;
constexpr auto ErrorProfilerDllNotFound = -6;
constexpr auto CLRErrorProfilerCouldNotBeAttached = -7;
constexpr auto ErrorCouldNotGetProcessBitness = -8;
constexpr auto ErrorProcessIs32Bit = -9;

int main(int argc, char* argv[])
{
    std::cout << "OctoAttach for .NET Framework!\n";
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
        Error(std::format("Could not open process ID: {0}.", pid));
        return ErrorCouldNotOpenProcess;
    }
    BOOL isWow64Process = false;
    PBOOL pIsWow64Process = &isWow64Process;
    if (!IsWow64Process(handle, pIsWow64Process))
    {
        Error("Could not obtain process bitness.");
        return ErrorCouldNotGetProcessBitness;
    }
    if (isWow64Process)
    {
        Error("Process is 32-bit. Currently only works with 64-bit processes.");
        return ErrorProcessIs32Bit;
    }
    ICLRMetaHost* _metahost = nullptr;
    HRESULT hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, reinterpret_cast<LPVOID*>(&_metahost));
    if (FAILED(hr))
    {
        Error("Could not obtain CLR Meta host");
        return CLRErrorNoMetaHost;
    }

    IEnumUnknown *enumIterator = nullptr;
    hr = _metahost->EnumerateLoadedRuntimes(handle, &enumIterator);
    if (FAILED(hr))
    {
        Error("Could not obtain Loaded runtimes enumerator.");
        return CLRErrorNoInstalledRuntimesFound;
    }
    ICLRRuntimeInfo *runtimeInfo = nullptr;
    ULONG retrievedNum{};
    hr = enumIterator->Next(1, reinterpret_cast<IUnknown**>(&runtimeInfo), &retrievedNum);
    while (retrievedNum > 0)
    {
        WCHAR runtimeVersion[255];
        ULONG versionInfoLen;
        hr = runtimeInfo->GetVersionString(runtimeVersion, &versionInfoLen);
        auto _version = std::wstring(runtimeVersion);
        const std::string version(_version.begin(), _version.end());
        std::cout << "Loaded runtime: " << version  << '\n';
        if (runtimeInfo)
        {
            break;
        }
        hr = enumIterator->Next(1, reinterpret_cast<IUnknown**>(&runtimeInfo), &retrievedNum);
    }

    if (!runtimeInfo)
    {
        Error("No runtime info loaded found. Currently only works for .NET Framework applications");
        return CLRErrorNoLoadedRuntimesFound;
    }

    ICLRProfiling* clrProfiling = nullptr;
    hr = runtimeInfo->GetInterface(CLSID_CLRProfiling, IID_ICLRProfiling, reinterpret_cast<LPVOID*>(&clrProfiling));
    if (FAILED(hr))
    {
        Error("Could not get CLRProfiling interface");
        return CLRErrorNoProfilingInterface;
    }

    static constexpr GUID CLSID_ProfilerCallback3 = { 0x4FD2ED52, 0x7731, 0x4b8d, { 0x94, 0x69, 0x03, 0xD2, 0xCC, 0x30, 0x86, 0xC5 } };

    std::wstring path = L".\\OctoProfiler.dll";

    if (!PathFileExistsW(path.c_str()))
    {
        Error(std::format(L"Did not find Profiler dll file. Make sure '{0}' is in the folder.", path));
        return ErrorProfilerDllNotFound;
    }

    hr = clrProfiling->AttachProfiler(pid, 2000, &CLSID_ProfilerCallback3, path.c_str(), nullptr, 0);
    if (FAILED(hr))
    {
        Error(std::format("Could not attach profiler. Hr = {0:X}", hr));
        return CLRErrorProfilerCouldNotBeAttached;
    }
    WaitForSingleObject(handle, INFINITE);
    return 0;

}
