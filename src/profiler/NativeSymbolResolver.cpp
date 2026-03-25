#include "NativeSymbolResolver.h"
#include "log.h"

NativeSymbolResolver::NativeSymbolResolver()
	: process_handle_(GetCurrentProcess()), initialized_(false) {}

NativeSymbolResolver::~NativeSymbolResolver()
{
	Cleanup();
}

bool NativeSymbolResolver::Initialize()
{
	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
	if (!SymInitialize(process_handle_, nullptr, TRUE))
	{
		Logger::Error(std::format("NativeSymbolResolver::Initialize failed with error {}", GetLastError()));
		return false;
	}
	initialized_ = true;
	Logger::DoLog("NativeSymbolResolver::Initialize succeeded");
	return true;
}

void NativeSymbolResolver::Cleanup()
{
	if (initialized_)
	{
		SymCleanup(process_handle_);
		initialized_ = false;
	}
}

std::optional<std::string> NativeSymbolResolver::ResolveAddress(UINT_PTR ip)
{
	if (!initialized_) return std::nullopt;

	{
		std::shared_lock lock(cache_mutex_);
		auto it = address_cache_.find(ip);
		if (it != address_cache_.end())
			return it->second;
	}

	std::string result;
	{
		std::lock_guard lock(sym_mutex_);

		alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
		auto symbol = reinterpret_cast<PSYMBOL_INFO>(buffer);
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = MAX_SYM_NAME;

		DWORD64 displacement = 0;
		if (!SymFromAddr(process_handle_, ip, &displacement, symbol))
			return std::nullopt;

		IMAGEHLP_MODULE64 module_info{};
		module_info.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
		if (SymGetModuleInfo64(process_handle_, ip, &module_info))
			result = std::format("{}!{}", module_info.ModuleName, symbol->Name);
		else
			result = symbol->Name;
	}

	{
		std::unique_lock lock(cache_mutex_);
		address_cache_[ip] = result;
	}

	return result;
}
