#include "NameResolver.h"


std::optional<std::wstring> NameResolver::ResolveFunctionName(FunctionID functionId) const {
	ModuleID moduleId;
	ClassID classId;
	mdTypeDef defToken;
	auto hr = pInfo->GetFunctionInfo(functionId, &classId, &moduleId, &defToken);
	if (FAILED(hr))
	{		
		return {};
	}

	IMetaDataImport* pIMDImport;
	hr = pInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, (IUnknown**)&pIMDImport);
	if (SUCCEEDED(hr))
	{
		WCHAR functionName[255];
		hr = pIMDImport->GetMethodProps(defToken,
			NULL,
			functionName,
			254,
			0,
			0,
			NULL,
			NULL,
			NULL,
			NULL);

		if (SUCCEEDED(hr))
		{						
			return std::optional<std::wstring>(std::wstring(functionName));
		}
	}

	return {};
}

std::optional<std::wstring> NameResolver::ResolveAssemblyName(AssemblyID assemblyId) const {
	WCHAR name[255];
	ULONG outNameLen;
	AppDomainID appDomainId;
	ModuleID moduleId;
	auto hr = pInfo->GetAssemblyInfo(assemblyId, 254, &outNameLen, name, &appDomainId, &moduleId);
	return std::optional<std::wstring>(std::wstring(name));
}