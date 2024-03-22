#include "OctoProfilerFactory.h"
#include "log.h"

inline std::optional<std::string> GetEnv(const char* key) {
	if (key == nullptr) {
		throw std::invalid_argument("Null pointer passed as environment variable name");
	}
	if (*key == '\0') {
		throw std::invalid_argument("Value requested for the empty-name environment variable");
	}
	char* buffer = nullptr;
	auto err = _dupenv_s(&buffer, NULL, key);
	if (buffer == nullptr) {
		return {};		
	}
	else {		
		return std::optional<std::string>(std::string { buffer });
	}
}

BOOL DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	return TRUE;
}
static OctoProfileFactory *factory;
extern "C" HRESULT STDMETHODCALLTYPE DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv)
{
	auto fileToLog = GetEnv("OCTO_PROFILER_FILE");
	Logger::DoLog("Log file: %s", fileToLog.value_or("console").c_str());

	Logger::DoLog("OctoProfiler::DllGetClassObject");

	if (ppv == nullptr)
	{
		return E_FAIL;
	}

	factory = new OctoProfileFactory();

	*ppv = factory;
	
	// for now return S_OK
	return S_OK;
}