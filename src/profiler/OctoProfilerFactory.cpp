#include "OctoProfilerFactory.h"

HRESULT __stdcall OctoProfileFactory::QueryInterface(REFIID riid, void** ppvObject)
{
	static const GUID CLSID_Profiler = { 0xdc27bf80, 0x3a36, 0x40d4, { 0x92, 0x78, 0x64, 0x15, 0x50, 0x8c, 0x4e, 0xd6 } };
	printf("OctoProfileFactory::QueryInterface");
	if (riid == CLSID_Profiler)
	{
		*ppvObject = &_profiler;
		return S_OK;
	}
	return E_NOTIMPL;
}

ULONG __stdcall OctoProfileFactory::AddRef(void)
{
	return 1;
}

ULONG __stdcall OctoProfileFactory::Release(void)
{
	return 0;
}

HRESULT __stdcall OctoProfileFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
{
	printf("OctoProfileFactory::CreateInstance");
	_profiler = new OctoProfiler();
	return S_OK;
}

HRESULT __stdcall OctoProfileFactory::LockServer(BOOL fLock)
{
	return E_NOTIMPL;
}
