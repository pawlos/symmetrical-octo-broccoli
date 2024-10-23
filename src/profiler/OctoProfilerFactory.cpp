#include "OctoProfilerFactory.h"

HRESULT __stdcall OctoProfilerFactory::QueryInterface(REFIID riid, void** ppvObject)
{	
	Logger::DoLog(std::format("OctoProfilerFactory::QueryInterface - {0}", format_iid(riid)));	
	static constexpr GUID Profiler_GUID = { 0x10B46309, 0xC972, 0x4F33, {0xB5,0xAB,0x5E,0x6E,0x3E,0xBA,0x2B,0x1A } };
	if (riid == Profiler_GUID)
	{
		
		*ppvObject = this;
		//profiler_->AddRef();	
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
	Logger::DoLog(std::format("OctoProfilerFactory::CreateInstance - {0}", format_iid(riid)));

	if (riid == IID_ICorProfilerCallback2 ||
		riid == IID_ICorProfilerCallback3)
	{
		profiler_ = (this->do_profile_enter_leave_ ? static_cast<ICorProfilerCallback3*>(
		new (std::nothrow) OctoProfilerEnterLeave()) : new (std::nothrow) OctoProfiler());
		profiler_->AddRef();
		*ppvObject = profiler_;
		return S_OK;
	}

	*ppvObject = this;	
	return S_OK;	
}

HRESULT __stdcall OctoProfilerFactory::LockServer(BOOL fLock)
{
	return S_OK;
}
