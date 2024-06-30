#include "OctoProfilerFactory.h"

HRESULT __stdcall OctoProfilerFactory::QueryInterface(REFIID riid, void** ppvObject)
{
	static constexpr GUID CLSID_ClassFactoryGuid = { 0x00000001, 0x0000, 0x0000, { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
	Logger::DoLog("OctoProfilerFactory::QueryInterface");
	if (riid == CLSID_ClassFactoryGuid)
	{
		*ppvObject = &profiler;
		this->AddRef();
		return S_OK;
	}
	*ppvObject = nullptr;
	return E_NOINTERFACE;
}

ULONG __stdcall OctoProfilerFactory::AddRef(void)
{
	return 1;
}

ULONG __stdcall OctoProfilerFactory::Release(void)
{
	return 0;
}

HRESULT __stdcall OctoProfilerFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
{
	Logger::DoLog(std::format("OctoProfilerFactory::CreateInstance - {0}", FormatIID(riid)));

	profiler = (this->m_doProfileEnterLeave ? (ICorProfilerCallback3*)new OctoProfilerEnterLeave() : new OctoProfiler());
	*ppvObject = profiler;
	return S_OK;
}

HRESULT __stdcall OctoProfilerFactory::LockServer(BOOL fLock)
{
	return S_OK;
}
