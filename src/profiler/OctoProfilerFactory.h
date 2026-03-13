#pragma once
#include "unknwn.h"
#include "OctoProfiler.h"
#include "OctoProfilerEnterLeave.h"

class OctoProfilerFactory : public IClassFactory
{
public:
	OctoProfilerFactory(bool doProfileEnterLeave, bool usePipe, uint32_t sampleRate = 10)
		: do_profile_enter_leave_(doProfileEnterLeave), use_pipe_(usePipe), sample_rate_(sampleRate) {}
	ICorProfilerCallback3* profiler_ = nullptr;
	bool do_profile_enter_leave_ = false;
	bool use_pipe_ = false;
	uint32_t sample_rate_ = 10;
	// Inherited via IClassFactory
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
	ULONG __stdcall AddRef() override;
	ULONG __stdcall Release() override;
	HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override;
	HRESULT __stdcall LockServer(BOOL fLock) override;
};