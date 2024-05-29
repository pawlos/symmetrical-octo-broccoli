#include "NameResolver.h"
#include <memory>
#include <format>


std::optional<std::wstring> NameResolver::ResolveFunctionNameWithFrameInfo(FunctionID functionId, COR_PRF_FRAME_INFO frameInfo) const
{
	ModuleID moduleId;
	ClassID classId;
	mdTypeDef defToken;
	CorElementType elementType;
	ClassID baseClassId;
	ULONG rank;
	ULONG32 pcTypeArgs;
	ClassID typeArgs[10];
	ThreadID threadId;
	auto hr = pInfo->GetFunctionInfo2(functionId, frameInfo, &classId, &moduleId, &defToken, 10, &pcTypeArgs, typeArgs);
	if (FAILED(hr))
	{
		return {};
	}

	hr = pInfo->GetCurrentThreadID(&threadId);
	std::shared_ptr<IMetaDataImport> imp = std::shared_ptr<IMetaDataImport>();
	hr = pInfo->GetModuleMetaData(moduleId, ofRead, IID_IMetaDataImport, reinterpret_cast<IUnknown**>(&imp));
	if (SUCCEEDED(hr))
	{
		WCHAR functionName[255];
		hr = imp.get()->GetMethodProps(defToken,
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
			hr = pInfo->IsArrayClass(classId, &elementType, &baseClassId, &rank);
			ClassID classToCheck = classId;
			if (hr == S_OK)
			{
				classToCheck = baseClassId;
			}
			auto className = this->ResolveTypeNameByClassId(classToCheck).value_or(L"<<empty>>");
			auto moduleName = this->ResolveModuleName(moduleId).value_or(L"");
			return moduleName + L";" + className + L";" + std::wstring(functionName) + L";" + std::format(L"{0}", threadId);
		}
	}

	return {};
}

std::optional<std::wstring> NameResolver::ResolveFunctionName(FunctionID functionId) const 
{
	ModuleID moduleId;
	ClassID classId;
	mdTypeDef defToken;
	
	auto hr = pInfo->GetFunctionInfo(functionId, &classId, &moduleId, &defToken);
	if (FAILED(hr))
	{		
		return {};
	}

	if (classId == 0)
	{
		return {};
	}
	
	std::shared_ptr<IMetaDataImport> imp = std::shared_ptr<IMetaDataImport>();
	hr = pInfo->GetModuleMetaData(moduleId, ofRead, IID_IMetaDataImport, reinterpret_cast<IUnknown**>(&imp));
	if (SUCCEEDED(hr))
	{
		WCHAR functionName[255];
		hr = imp.get()->GetMethodProps(defToken,
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
			CorElementType elementType;
			ClassID baseClassId;
			ULONG rank;
			hr = pInfo->IsArrayClass(classId, &elementType, &baseClassId, &rank);
			ClassID classToCheck = classId;
			if (hr == S_OK)
			{
				classToCheck = baseClassId;
			}
			auto className = this->ResolveTypeNameByClassId(classToCheck).value_or(L"<<empty>>");
			auto moduleName = this->ResolveModuleName(moduleId).value_or(L"");
			return moduleName + L";" + className + L";" + std::wstring(functionName);
		}
	}

	return {};
}

std::optional<std::wstring> NameResolver::ResolveAssemblyName(AssemblyID assemblyId) const 
{
	WCHAR name[255];
	ULONG outNameLen;
	AppDomainID appDomainId;
	ModuleID moduleId;
	auto hr = pInfo->GetAssemblyInfo(assemblyId, 254, &outNameLen, name, &appDomainId, &moduleId);
	return std::wstring(name);
}

std::optional<std::wstring> NameResolver::ResolveAppDomainName(AppDomainID appDomainId) const
{
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

std::optional<std::wstring> NameResolver::ResolveTypeNameByClassIdWithExistingMetaData(ClassID classId, IMetaDataImport* pIMDImport) const
{
	ModuleID moduleId;
	mdTypeDef defToken;
	CorElementType elementType;
	ClassID baseClassId;
	ULONG rank;
	ClassID classToCheck = classId;
	bool isArray = false;
	auto hr = pInfo->IsArrayClass(classToCheck, &elementType, &baseClassId, &rank);
	if (hr == S_OK)
	{
		classToCheck = baseClassId;
		isArray = true;
	}
	hr = pInfo->GetClassIDInfo(classToCheck, &moduleId, &defToken);
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

		return std::wstring(typeName) + (isArray ? L"[]" : L"");
	}
	return {};
}

std::optional<std::wstring> NameResolver::ResolveTypeNameByClassId(ClassID classId) const
{	
	ModuleID moduleId;
	mdTypeDef defToken;				
	auto hr = pInfo->GetClassIDInfo(
		classId,
		&moduleId,
		&defToken);
	if (SUCCEEDED(hr))
	{
		IMetaDataImport* pIMDImport = nullptr;
		hr = pInfo->GetModuleMetaData(moduleId, ofRead, IID_IMetaDataImport, reinterpret_cast<IUnknown**>(& pIMDImport));
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

std::optional<std::wstring> NameResolver::ResolveTypeNameByObjectId(ObjectID objectId) const
{
	ClassID classId;
	auto hr = pInfo->GetClassFromObject(objectId, &classId);
	if (SUCCEEDED(hr))
	{
		return this->ResolveTypeNameByObjectIdAndClassId(objectId, classId);
	}
	return{};
}

std::optional<std::wstring> NameResolver::ResolveTypeNameByObjectIdAndClassId(ObjectID objectId, ClassID classId) const
{
	CorElementType elementType;
	ClassID baseClassId;
	ULONG rank;
	auto hr = pInfo->IsArrayClass(classId, &elementType, &baseClassId, &rank);
	if (hr == S_OK)
	{
		auto name = this->ResolveTypeNameByClassId(baseClassId);
		if (!name) return name;
		return name.value() + L"[]";
	}
	else
	{
		return this->ResolveTypeNameByClassId(classId);
	}
}

std::optional<std::wstring> NameResolver::ResolveModuleName(ModuleID moduleId) const
{
	WCHAR moduleName[1026];
	AssemblyID assemblyId;
	LPCBYTE baseLoadAddress;
	ULONG size;
	auto hr = pInfo->GetModuleInfo(moduleId, &baseLoadAddress, 1024, &size, moduleName, &assemblyId);
	return std::wstring(moduleName);
}