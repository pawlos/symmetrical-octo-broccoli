#include "NameResolver.h"
#include <memory>
#include <format>


std::optional<std::wstring> NameResolver::ResolveFunctionNameWithFrameInfo(
	const FunctionID function_id, const COR_PRF_FRAME_INFO frame_info) const
{
    ModuleID module_id;
    ClassID class_id;
    mdTypeDef def_token;
    CorElementType element_type;
    ClassID base_class_id;
    ULONG rank;
    ULONG32 pc_type_args;
    ClassID type_args[10];
    ThreadID thread_id;
    auto hr = profiler_info_->GetFunctionInfo2(function_id, frame_info, &class_id, &module_id, &def_token, 10,
                                               &pc_type_args, type_args);
    if (FAILED(hr))
    {
        return {};
    }

    hr = profiler_info_->GetCurrentThreadID(&thread_id);
    if (FAILED(hr))
    {
        return {};
    }
    const auto thread_name = ResolveCurrentThreadName();

    auto imp = std::shared_ptr<IMetaDataImport>();
    hr = profiler_info_->GetModuleMetaData(module_id, ofRead, IID_IMetaDataImport, reinterpret_cast<IUnknown**>(&imp));
    if (SUCCEEDED(hr))
    {
        WCHAR function_name[255];
        hr = imp->GetMethodProps(def_token,
                                 nullptr,
                                 function_name,
                                 254,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr);

        if (SUCCEEDED(hr))
        {
            hr = profiler_info_->IsArrayClass(class_id, &element_type, &base_class_id, &rank);
            ClassID class_to_check = class_id;
            if (hr == S_OK)
            {
                class_to_check = base_class_id;
            }
            const auto class_name = this->ResolveTypeNameByClassId(class_to_check).value_or(L"<<empty>>");
            const auto module_name = this->ResolveModuleName(module_id).value_or(L"");
            return module_name + L";" + class_name + L";" + std::wstring(function_name) + L";" + thread_name.value_or(
                std::format(L"{0}", thread_id));
        }
    }

    return {};
}

std::optional<std::wstring> NameResolver::ResolveFunctionName(const FunctionID function_id) const
{
    ModuleID module_id;
    ClassID class_id;
    mdTypeDef def_token;

    auto hr = profiler_info_->GetFunctionInfo(function_id, &class_id, &module_id, &def_token);
    if (FAILED(hr) || class_id == 0)
    {
        return {};
    }

    auto imp = std::shared_ptr<IMetaDataImport>();
    hr = profiler_info_->GetModuleMetaData(module_id, ofRead, IID_IMetaDataImport, reinterpret_cast<IUnknown**>(&imp));
    if (SUCCEEDED(hr))
    {
        WCHAR functionName[255];
        hr = imp->GetMethodProps(def_token,
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
            CorElementType element_type;
            ClassID base_class_id;
            ULONG rank;
            hr = profiler_info_->IsArrayClass(class_id, &element_type, &base_class_id, &rank);
            ClassID classToCheck = class_id;
            if (hr == S_OK)
            {
                classToCheck = base_class_id;
            }
            const auto className = this->ResolveTypeNameByClassId(classToCheck).value_or(L"<<empty>>");
            const auto moduleName = this->ResolveModuleName(module_id).value_or(L"");
            return moduleName + L";" + className + L";" + std::wstring(functionName);
        }
    }

    return {};
}

std::optional<std::wstring> NameResolver::ResolveAssemblyName(const AssemblyID assembly_id) const
{
    WCHAR name[255];
    ULONG out_name_len;
    AppDomainID app_domain_id;
    ModuleID module_id;
    if (SUCCEEDED(profiler_info_->GetAssemblyInfo(assembly_id, 254, &out_name_len, name, &app_domain_id, &module_id)))
        return std::wstring(name);
    return {};
}

std::optional<std::wstring> NameResolver::ResolveAppDomainName(const AppDomainID app_domain_id) const
{
    DWORD app_domain_name_count;
    WCHAR app_domain_name[255];
    ProcessID process_id;
    auto hr = profiler_info_->GetAppDomainInfo(app_domain_id, 255, &app_domain_name_count, app_domain_name, &process_id);
    if (FAILED(hr))
    {
        return {};
    }

    return std::wstring(app_domain_name);
}

std::optional<std::wstring> NameResolver::ResolveTypeNameByClassIdWithExistingMetaData(
	const ClassID class_id, IMetaDataImport* pIMDImport) const
{
    ModuleID moduleId;
    mdTypeDef defToken;
    CorElementType elementType;
    ClassID baseClassId;
    ULONG rank;
    ClassID classToCheck = class_id;
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
        if (SUCCEEDED(pIMDImport->GetTypeDefProps(defToken,
            typeName,
            510,
            &typedefnamesize,
            &typedefflags,
            &extends)))
        {
            return std::wstring(typeName) + (isArray ? L"[]" : L"");
        }
    }
    return {};
}

std::optional<std::wstring> NameResolver::ResolveTypeNameByClassId(const ClassID class_id) const
{
    ModuleID moduleId;
    mdTypeDef defToken;
    auto hr = profiler_info_->GetClassIDInfo(
        class_id,
        &moduleId,
        &defToken);
    if (SUCCEEDED(hr))
    {
        IMetaDataImport* pIMDImport = nullptr;
        hr = profiler_info_->GetModuleMetaData(moduleId, ofRead, IID_IMetaDataImport,
                                               reinterpret_cast<IUnknown**>(&pIMDImport));
        if (SUCCEEDED(hr))
        {
            ULONG typedefnamesize;
            DWORD typedefflags;
            mdToken extends;
            WCHAR typeName[512];
            if (SUCCEEDED(pIMDImport->GetTypeDefProps(defToken,
                                             typeName,
                                             510,
                                             &typedefnamesize,
                                             &typedefflags,
                                             &extends)))
            {
                return std::wstring(typeName);
            }
        }
    }
    return {};
}

std::optional<std::wstring> NameResolver::ResolveTypeNameByObjectId(const ObjectID object_id) const
{
    ClassID class_id;
    auto hr = profiler_info_->GetClassFromObject(object_id, &class_id);
    if (SUCCEEDED(hr))
    {
        return this->ResolveTypeNameByObjectIdAndClassId(object_id, class_id);
    }
    return {};
}

std::optional<std::wstring> NameResolver::ResolveTypeNameByObjectIdAndClassId(ObjectID object_id, const ClassID class_id) const
{
    CorElementType element_type;
    ClassID base_class_id;
    ULONG rank;
    if (profiler_info_->IsArrayClass(class_id, &element_type, &base_class_id, &rank) == S_OK)
    {
        auto name = this->ResolveTypeNameByClassId(base_class_id);
        if (!name) return name;
        return name.value() + L"[]";
    }
    return this->ResolveTypeNameByClassId(class_id);
}

std::optional<std::wstring> NameResolver::ResolveModuleName(const ModuleID module_id) const
{
    WCHAR module_name[1026];
    AssemblyID assembly_id;
    LPCBYTE base_load_address;
    ULONG size;
    if (FAILED(profiler_info_->GetModuleInfo(module_id, &base_load_address, 1024, &size, module_name, &assembly_id)))
    {
        return {};
    }
    return std::wstring(module_name);
}

std::optional<std::wstring> NameResolver::ResolveCurrentThreadName()
{
    const HANDLE thread = GetCurrentThread();
    PWSTR thread_description = nullptr;
    if (SUCCEEDED(GetThreadDescription(thread, &thread_description)))
    {
        return thread_description;
    }
    return {};
}

std::optional<std::wstring> NameResolver::ResolveNetRuntimeVersion()
{
    USHORT clr_runtime_id{ 0 };
    COR_PRF_RUNTIME_TYPE runtime_type{};
    USHORT major_version{ 0 };
    USHORT minor_version{ 0 };
    USHORT build_number{ 0 };
    USHORT qfe_version{ 0 };
    ULONG version_string_len{ 0 };
    WCHAR version_string[256];
    auto hr = profiler_info_->GetRuntimeInformation(
        &clr_runtime_id,
        &runtime_type,
        &major_version,
        &minor_version,
        &build_number,
        &qfe_version,
        255,
        &version_string_len,
        version_string);
    if (FAILED(hr))
    {
        return {};
    }

    return std::wstring(version_string);
}
