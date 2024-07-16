#include "OctoProfilerFactory.h"
#include "log.h"

static OctoProfilerFactory* factory;

inline std::optional<std::string> get_env(const char* key)
{
	if (key == nullptr) {
		throw std::invalid_argument("Null pointer passed as environment variable name");
	}
	if (*key == '\0') {
		throw std::invalid_argument("Value requested for the empty-name environment variable");
	}
	char* buffer = nullptr;
	if (_dupenv_s(&buffer, nullptr, key) != 0 || buffer == nullptr) {
		return {};
	}
	return { std::string { buffer } };
}

inline bool to_bool(const std::string& str)
{
	std::istringstream is(str);
	bool b;
	is >> std::boolalpha >> b;
	return b;
}

BOOL DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	return TRUE;
}

extern "C" HRESULT STDMETHODCALLTYPE DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
	const auto file_to_log = get_env("OCTO_PROFILER_FILE");
	const auto include_ts = to_bool(get_env("OCTO_LOGGER_INCLUDE_TS").value_or("true"));
	const auto logger = file_to_log.has_value() ? static_cast<Logger *>(new FileLogger(file_to_log.value(), include_ts)) : static_cast<Logger *>(new StdOutLogger());
	Logger::initialize(logger);
	Logger::DoLog(std::format("Log file: {0}", file_to_log.value_or("console")));
	Logger::DoLog("OctoProfiler::DllGetClassObject");

	auto doProfileEnterLeave = to_bool(get_env("OCTO_MONITOR_ENTERLEAVE").value_or("false"));
	Logger::DoLog(std::format("OctoProfiler::MonitorEnterLeave: {0}", doProfileEnterLeave));

	if (ppv == nullptr)
	{
		return E_FAIL;
	}

	factory = new OctoProfilerFactory(doProfileEnterLeave);

	*ppv = factory;

	return S_OK;
}