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
	auto hr = profiler_info_->GetFunctionInfo2(functionId, frameInfo, &classId, &moduleId, &defToken, 10, &pcTypeArgs, typeArgs);
	if (FAILED(hr))
	{
		return {};
	}

	hr = profiler_info_->GetCurrentThreadID(&threadId);
	auto threadName = ResolveCurrentThreadName();

	std::shared_ptr<IMetaDataImport> imp = std::shared_ptr<IMetaDataImport>();
	hr = profiler_info_->GetModuleMetaData(moduleId, ofRead, IID_IMetaDataImport, reinterpret_cast<IUnknown**>(&imp));
	if (SUCCEEDED(hr))
	{
		WCHAR functionName[255];
		hr = imp->GetMethodProps(defToken,
			nullptr,
			functionName,
			254,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr);

		if (SUCCEEDED(hr))
		{
			hr = profiler_info_->IsArrayClass(classId, &elementType, &baseClassId, &rank);
			ClassID classToCheck = classId;
			if (hr == S_OK)
			{
				classToCheck = baseClassId;
			}
			auto className = this->ResolveTypeNameByClassId(classToCheck).value_or(L"<<empty>>");
			auto moduleName = this->ResolveModuleName(moduleId).value_or(L"");
			return moduleName + L";" + className + L";" + std::wstring(functionName) + L";" + threadName.value_or(std::format(L"{0}", threadId));
		}
	}

	return {};
}

std::optional<std::wstring> NameResolver::ResolveFunctionName(FunctionID functionId) const
{
	ModuleID moduleId;
	ClassID classId;
	mdTypeDef defToken;

	auto hr = profiler_info_->GetFunctionInfo(functionId, &classId, &moduleId, &defToken);
	if (FAILED(hr))
	{
		return {};
	}

	if (classId == 0)
	{
		return {};
	}

	auto imp = std::shared_ptr<IMetaDataImport>();
	hr = profiler_info_->GetModuleMetaData(moduleId, ofRead, IID_IMetaDataImport, reinterpret_cast<IUnknown**>(&imp));
	if (SUCCEEDED(hr))
	{
		WCHAR functionName[255];
		hr = imp->GetMethodProps(defToken,
			nullptr,
			functionName,
			254,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr);

		if (SUCCEEDED(hr))
		{
			CorElementType elementType;
			ClassID baseClassId;
			ULONG rank;
			hr = profiler_info_->IsArrayClass(classId, &elementType, &baseClassId, &rank);
			ClassID classToCheck = classId;
			if (hr == S_OK)
			{
				classToCheck = baseClassId;
			}
			const auto className = this->ResolveTypeNameByClassId(classToCheck).value_or(L"<<empty>>");
			const auto moduleName = this->ResolveModuleName(moduleId).value_or(L"");
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
	const auto hr = profiler_info_->GetAssemblyInfo(assemblyId, 254, &outNameLen, name, &appDomainId, &moduleId);
	if (SUCCEEDED(hr))
		return std::wstring(name);
	return {};
}

std::optional<std::wstring> NameResolver::ResolveAppDomainName(AppDomainID appDomainId) const
{
	DWORD appDomainNameCount;
	WCHAR appDomainName[255];
	ProcessID processId;
	auto hr = profiler_info_->GetAppDomainInfo(appDomainId, 255, &appDomainNameCount, appDomainName, &processId);
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
	auto hr = profiler_info_->IsArrayClass(classToCheck, &elementType, &baseClassId, &rank);
	if (hr == S_OK)
	{
		classToCheck = baseClassId;
		isArray = true;
	}
	hr = profiler_info_->GetClassIDInfo(classToCheck, &moduleId, &defToken);
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
	auto hr = profiler_info_->GetClassIDInfo(
		classId,
		&moduleId,
		&defToken);
	if (SUCCEEDED(hr))
	{
		IMetaDataImport* pIMDImport = nullptr;
		hr = profiler_info_->GetModuleMetaData(moduleId, ofRead, IID_IMetaDataImport, reinterpret_cast<IUnknown**>(& pIMDImport));
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
	auto hr = profiler_info_->GetClassFromObject(objectId, &classId);
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
	auto hr = profiler_info_->IsArrayClass(classId, &elementType, &baseClassId, &rank);
	if (hr == S_OK)
	{
		auto name = this->ResolveTypeNameByClassId(baseClassId);
		if (!name) return name;
		return name.value() + L"[]";
	}
	return this->ResolveTypeNameByClassId(classId);
}

std::optional<std::wstring> NameResolver::ResolveModuleName(ModuleID moduleId) const
{
	WCHAR moduleName[1026];
	AssemblyID assemblyId;
	LPCBYTE baseLoadAddress;
	ULONG size;
	auto hr = profiler_info_->GetModuleInfo(moduleId, &baseLoadAddress, 1024, &size, moduleName, &assemblyId);
	return std::wstring(moduleName);
}

std::optional<std::wstring> NameResolver::ResolveCurrentThreadName()
{
	HANDLE thread = GetCurrentThread();
	PWSTR threadDescription = nullptr;
	auto hr = GetThreadDescription(thread, &threadDescription);
	if (SUCCEEDED(hr))
	{
		return threadDescription;
	}
	return {};
}