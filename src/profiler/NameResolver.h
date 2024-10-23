#pragma once
#include <string>
#include <cor.h>
#include <corprof.h>
#include <optional>

class NameResolver
{
public:
	NameResolver(ICorProfilerInfo5* profiler_info): profiler_info_(profiler_info) {}
	std::optional<std::wstring> ResolveFunctionName(FunctionID function_id) const;
	std::optional<std::wstring> ResolveFunctionNameWithFrameInfo(FunctionID function_id, COR_PRF_FRAME_INFO frame_info) const;
	std::optional<std::wstring> ResolveAssemblyName(AssemblyID assembly_id) const;
	std::optional<std::wstring> ResolveAppDomainName(AppDomainID app_domain_id) const;
	std::optional<std::wstring> ResolveModuleName(ModuleID module_id) const;
	std::optional<std::wstring> ResolveTypeNameByClassId(ClassID class_id) const;
	std::optional<std::wstring> ResolveTypeNameByObjectId(ObjectID object_id) const;
	std::optional<std::wstring> ResolveTypeNameByObjectIdAndClassId(ObjectID object_id, ClassID class_id) const;
	std::optional<std::wstring> ResolveNetRuntimeVersion();
	static std::optional<std::wstring> ResolveCurrentThreadName();
private:
	std::optional<std::wstring> ResolveTypeNameByClassIdWithExistingMetaData(ClassID class_id, IMetaDataImport*) const;
	ICorProfilerInfo5* profiler_info_;
};

