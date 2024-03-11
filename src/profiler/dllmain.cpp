#include "octoprofiler.h"

BOOL DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	return TRUE;
}

extern "C" HRESULT STDMETHODCALLTYPE DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv)
{
	// {DC27BF80-3A36-40D4-9278-6415508C4ED6}
	static const GUID CLSID_Profiler = { 0xdc27bf80, 0x3a36, 0x40d4, { 0x92, 0x78, 0x64, 0x15, 0x50, 0x8c, 0x4e, 0xd6 } };

	if (ppv == nullptr || rclsid != CLSID_Profiler)
	{
		return E_FAIL;
	}

	auto profiler = new OctoProfiler();

	profiler->QueryInterface(riid, ppv);

	// for now return S_OK
	return S_OK;
}