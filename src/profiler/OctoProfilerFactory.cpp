#include "OctoProfilerFactory.h"

HRESULT __stdcall OctoProfileFactory::QueryInterface(REFIID riid, void** ppvObject)
{	
	static const GUID CLSID_ClassFactoryGuid = { 0x00000001, 0x0000, 0x0000, { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
	Logger::DoLog("OctoProfileFactory::QueryInterface");
	if (riid == CLSID_ClassFactoryGuid)
	{
		*ppvObject = &profiler;
		this->AddRef();
		return S_OK;
	}	
	*ppvObject = nullptr;
	return E_NOINTERFACE;
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
	Logger::DoLog("OctoProfileFactory::CreateInstance");
	profiler = new OctoProfiler();
	*ppvObject = profiler;
	return S_OK;
}

HRESULT __stdcall OctoProfileFactory::LockServer(BOOL fLock)
{
	return S_OK;
}
