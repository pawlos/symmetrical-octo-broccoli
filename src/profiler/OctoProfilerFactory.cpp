#include "OctoProfilerFactory.h"

HRESULT __stdcall OctoProfilerFactory::QueryInterface(REFIID riid, void** ppvObject)
{
	static constexpr GUID CLSID_ClassFactoryGuid = { 0x00000001, 0x0000, 0x0000, { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
	Logger::DoLog("OctoProfilerFactory::QueryInterface");
	if (riid == CLSID_ClassFactoryGuid)
	{
		*ppvObject = &profiler_;
		this->AddRef();
		return S_OK;
	}
	*ppvObject = nullptr;
	return E_NOINTERFACE;
}

ULONG __stdcall OctoProfilerFactory::AddRef()
{
	return 1;
}

ULONG __stdcall OctoProfilerFactory::Release()
{
	return 0;
}

HRESULT __stdcall OctoProfilerFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
{
	Logger::DoLog(std::format("OctoProfilerFactory::CreateInstance - {0}", FormatIID(riid)));

	profiler_ = (this->do_profile_enter_leave_ ? static_cast<ICorProfilerCallback3*>(new OctoProfilerEnterLeave()) : new OctoProfiler());
	*ppvObject = profiler_;
	return S_OK;
}

HRESULT __stdcall OctoProfilerFactory::LockServer(BOOL fLock)
{
	return S_OK;
}
