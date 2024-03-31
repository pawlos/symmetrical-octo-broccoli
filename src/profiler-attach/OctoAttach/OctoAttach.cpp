// OctoAttach.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <sstream>
#include <metahost.h>
#include <wchar.h>

int main(int argc, char* argv[])
{
    std::cout << "OctoAttach!\n";
    if (argc <= 1)
    {
        std::cout << "Invalid argument count\n";
        return -1;
    }

    auto pid = std::stoi(argv[1]);

    std::cout << "PID: " << pid << "\n";
    ICLRMetaHost* _metahost = nullptr;
    HRESULT hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID*)&_metahost);
    if (FAILED(hr))
    {
        std::cout << "Could not obtain CLR Mata host.\n";
        return -2;
    }

    ICLRRuntimeInfo* _runtimeinfo = nullptr;
    hr = _metahost->GetRuntime(L"v4.0.30319", IID_ICLRRuntimeInfo, (LPVOID*)&_runtimeinfo);
    if (FAILED(hr))
    {
        std::cout << "Could not obtain CLR Runtime info.\n";
        return -3;
    }

    ICLRProfiling* _clrProfiling = nullptr;
    hr = _runtimeinfo->GetInterface(CLSID_CLRProfiling, IID_ICLRProfiling, (LPVOID*)&_clrProfiling);
    return 0;

}
