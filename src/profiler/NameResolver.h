#pragma once
#include <string>
#include <cor.h>
#include <corprof.h>
#include <optional>

class NameResolver
{
public:
	NameResolver(ICorProfilerInfo5* pInfo): pInfo(pInfo) {}
	std::optional<std::wstring> ResolveFunctionName(FunctionID functionId) const;
	std::optional<std::wstring> ResolveFunctionNameWithFrameInfo(FunctionID functionId, COR_PRF_FRAME_INFO frameInfo) const;
	std::optional<std::wstring> ResolveAssemblyName(AssemblyID assemblyId) const;
	std::optional<std::wstring> ResolveAppDomainName(AppDomainID appDomainId) const;
	std::optional<std::wstring> ResolveModuleName(ModuleID moduleId) const;
	std::optional<std::wstring> ResolveTypeNameByClassId(ClassID classId) const;
	std::optional<std::wstring> ResolveTypeNameByObjectId(ObjectID objectId) const;
	std::optional<std::wstring> ResolveTypeNameByObjectIdAndClassId(ObjectID objectId, ClassID classId) const;
private:
	std::optional<std::wstring> ResolveTypeNameByClassIdWithExistingMetaData(ClassID classId, IMetaDataImport*) const;
	ICorProfilerInfo5* pInfo;
};

