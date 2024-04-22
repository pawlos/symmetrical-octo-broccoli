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

	IMetaDataImport* pIMDImport = nullptr;
	hr = pInfo->GetModuleMetaData(moduleId, ofRead, IID_IMetaDataImport, (IUnknown**)&pIMDImport);
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
			auto className = this->ResolveTypeNameByClassId(classId);
			if (className)
			{
				return className.value() + L"." + std::wstring(functionName);
			}
			else
			{
				return std::wstring(functionName);
			}
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
	return std::wstring(name);
}

std::optional<std::wstring> NameResolver::ResolveAppDomainName(AppDomainID appDomainId) const {
	DWORD appDomainNameCount;
	WCHAR appDomainName[255];
	ProcessID processId;
	auto hr = pInfo->GetAppDomainInfo(appDomainId, 255, &appDomainNameCount, appDomainName, &processId);
	if (FAILED(hr))
	{		
		return {};
	}

	return std::wstring(appDomainName);
}

std::optional<std::wstring> NameResolver::ResolveTypeNameByClassId(ClassID classId) const 
{	
	ModuleID moduleId;
	mdTypeDef defToken;
	auto hr = pInfo->GetClassIDInfo(classId, &moduleId, &defToken);
	if (SUCCEEDED(hr))
	{
		IMetaDataImport* pIMDImport = nullptr;
		hr = pInfo->GetModuleMetaData(moduleId, ofRead, IID_IMetaDataImport, (IUnknown**)&pIMDImport);
		if (SUCCEEDED(hr))
		{
			ULONG typedefnamesize;
			DWORD typedefflags;
			mdToken extends;
			WCHAR typeName[512];
			hr = pIMDImport->GetTypeDefProps(defToken,
				typeName,
				510,
				&typedefnamesize,
				&typedefflags,
				&extends);

			return std::wstring(typeName);
		}
	}
	return {};
}

std::optional<std::wstring> NameResolver::ResolveTypeNameByObjectId(ObjectID objectId) const {
	ClassID classId;
	auto hr = pInfo->GetClassFromObject(objectId, &classId);
	if (SUCCEEDED(hr))
	{
		CorElementType elementType;
		ClassID baseClassId;
		ULONG rank;
		hr = pInfo->IsArrayClass(classId, &elementType, &baseClassId, &rank);
		if (hr == S_OK)
		{
			auto name = this->ResolveTypeNameByClassId(baseClassId);
			if (!name) return name;
			return name.value() + L"[]";
		}
		else
			return this->ResolveTypeNameByClassId(classId);
	}
	return{};
}

std::optional<std::wstring> NameResolver::ResolveModuleName(ModuleID moduleId) const {
	WCHAR moduleName[512];
	AssemblyID assemblyId;
	LPCBYTE baseLoadAddress;
	ULONG size;
	auto hr = pInfo->GetModuleInfo(moduleId, &baseLoadAddress, 510, &size, moduleName, &assemblyId);

	return std::wstring(moduleName);
}