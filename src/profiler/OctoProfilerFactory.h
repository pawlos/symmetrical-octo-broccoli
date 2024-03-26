#pragma once
#include "unknwn.h"
#include "OctoProfiler.h"
#include <stdio.h>

class OctoProfileFactory : public IClassFactory
{
public:
	OctoProfiler* profiler = nullptr;
private:	
	// Inherited via IClassFactory
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
	ULONG __stdcall AddRef(void) override;
	ULONG __stdcall Release(void) override;
	HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override;
	HRESULT __stdcall LockServer(BOOL fLock) override;
};