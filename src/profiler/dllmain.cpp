#include "OctoProfilerFactory.h"

BOOL DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	return TRUE;
}
static OctoProfileFactory *factory;
extern "C" HRESULT STDMETHODCALLTYPE DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv)
{
	printf("OctoProfiler::DllGetClassObject\n");

	if (ppv == nullptr)
	{
		return E_FAIL;
	}

	factory = new OctoProfileFactory();

	*ppv = factory;
	
	// for now return S_OK
	return S_OK;
}