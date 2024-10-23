#include "OctoProfilerFactory.h"
#include "log.h"

static OctoProfilerFactory* factory = nullptr;

inline static std::optional<std::string> get_env(const char* key)
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

inline static bool to_bool(const std::string& str)
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

_Check_return_
extern "C" HRESULT STDMETHODCALLTYPE DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv)
{
	const auto file_to_log = get_env("OCTO_PROFILER_FILE");
	const auto include_ts = to_bool(get_env("OCTO_LOGGER_INCLUDE_TS").value_or("true"));
	const auto logger = file_to_log.has_value() ? static_cast<Logger *>(new FileLogger(file_to_log.value(), include_ts)) : static_cast<Logger *>(new StdOutLogger());
	Logger::initialize(logger);
	Logger::DoLog(std::format("Log file: {0}", file_to_log.value_or("console")));
	Logger::DoLog(std::format("OctoProfiler::DllGetClassObject {0} {1}", format_iid(rclsid), format_iid(riid)));

	auto doProfileEnterLeave = to_bool(get_env("OCTO_MONITOR_ENTERLEAVE").value_or("false"));
	Logger::DoLog(std::format("OctoProfiler::MonitorEnterLeave: {0}", doProfileEnterLeave));

	auto hr = E_FAIL;
	if (ppv == nullptr)
	{
		return hr;
	}
	
	static constexpr GUID CLSID_ClassFactoryGuid = { 0x00000001, 0x0000, 0x0000, { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
	if (riid == CLSID_ClassFactoryGuid)
	{
		factory = new (std::nothrow) OctoProfilerFactory(doProfileEnterLeave);
		if (factory)
		{
			factory->AddRef();
			hr = factory->QueryInterface(rclsid, ppv);			
		}
	}

	return hr;
}