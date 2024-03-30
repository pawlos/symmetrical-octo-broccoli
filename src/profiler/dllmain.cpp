#include "OctoProfilerFactory.h"
#include "log.h"

static OctoProfileFactory* factory;

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

extern "C" HRESULT STDMETHODCALLTYPE DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
	auto fileToLog = GetEnv("OCTO_PROFILER_FILE");	
	auto logger = fileToLog.has_value() ? static_cast<Logger *>(new FileLogger(fileToLog.value())) : static_cast<Logger *>(new StdOutLogger());		
	Logger::Initialize(logger);
	Logger::DoLog(std::format("Log file: {0}", fileToLog.value_or("console")));
	Logger::DoLog("OctoProfiler::DllGetClassObject");

	if (ppv == nullptr)
	{
		return E_FAIL;
	}

	factory = new OctoProfileFactory();

	*ppv = factory;
		
	return S_OK;
}