#pragma once
#include "unknwn.h"
#include "OctoProfiler.h"
#include "OctoProfilerEnterLeave.h"

class OctoProfilerFactory : public IClassFactory
{
public:
	ICorProfilerCallback3* profiler = nullptr;
	OctoProfilerFactory(bool doProfileEnterLeave) : m_doProfileEnterLeave(doProfileEnterLeave) {}
private:
	bool m_doProfileEnterLeave = false;
	// Inherited via IClassFactory
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
	ULONG __stdcall AddRef(void) override;
	ULONG __stdcall Release(void) override;
	HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override;
	HRESULT __stdcall LockServer(BOOL fLock) override;
};