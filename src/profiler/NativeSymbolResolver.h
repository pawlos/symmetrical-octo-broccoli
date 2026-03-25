#pragma once
#include <Windows.h>
#include <DbgHelp.h>
#include <string>
#include <optional>
#include <map>
#include <shared_mutex>
#include <mutex>

class NativeSymbolResolver
{
public:
	NativeSymbolResolver();
	~NativeSymbolResolver();

	bool Initialize();
	void Cleanup();
	std::optional<std::string> ResolveAddress(UINT_PTR ip);

private:
	HANDLE process_handle_;
	bool initialized_;
	std::map<UINT_PTR, std::string> address_cache_;
	std::shared_mutex cache_mutex_;
	std::mutex sym_mutex_;
};
