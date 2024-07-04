#pragma once
#include "unknwn.h"
#include "OctoProfiler.h"
#include "OctoProfilerEnterLeave.h"

class OctoProfilerFactory : public IClassFactory
{
public:
	OctoProfilerFactory(bool doProfileEnterLeave) : do_profile_enter_leave_(doProfileEnterLeave) {}
private:
	ICorProfilerCallback3* profiler_ = nullptr;
	bool do_profile_enter_leave_ = false;
	// Inherited via IClassFactory
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
	ULONG __stdcall AddRef() override;
	ULONG __stdcall Release() override;
	HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override;
	HRESULT __stdcall LockServer(BOOL fLock) override;
};