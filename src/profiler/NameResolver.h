#pragma once
#include <string>
#include <cor.h>
#include <corprof.h>
#include <optional>

class NameResolver
{
public:
	NameResolver(ICorProfilerInfo2* pInfo): pInfo(pInfo) {}
	std::optional<std::wstring> ResolveFunctionName(FunctionID functionId) const;
	std::optional<std::wstring> ResolveAssemblyName(AssemblyID assemblyId) const;
	std::optional<std::wstring> ResolveAppDomainName(AppDomainID appDomainId) const;
private:
	ICorProfilerInfo2* pInfo;
};

