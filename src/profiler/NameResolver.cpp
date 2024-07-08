#include "NameResolver.h"
#include <memory>
#include <format>


std::optional<std::wstring> NameResolver::ResolveFunctionNameWithFrameInfo(
    FunctionID functionId, COR_PRF_FRAME_INFO frameInfo) const
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
    auto hr = profiler_info_->GetFunctionInfo2(functionId, frameInfo, &class_id, &module_id, &def_token, 10,
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
    auto threadName = ResolveCurrentThreadName();

    std::shared_ptr<IMetaDataImport> imp = std::shared_ptr<IMetaDataImport>();
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
            hr = profiler_info_->IsArrayClass(class_id, &element_type, &base_class_id, &rank);
            ClassID classToCheck = class_id;
            if (hr == S_OK)
            {
                classToCheck = base_class_id;
            }
            auto className = this->ResolveTypeNameByClassId(classToCheck).value_or(L"<<empty>>");
            auto moduleName = this->ResolveModuleName(module_id).value_or(L"");
            return moduleName + L";" + className + L";" + std::wstring(functionName) + L";" + threadName.value_or(
                std::format(L"{0}", thread_id));
        }
    }

    return {};
}

std::optional<std::wstring> NameResolver::ResolveFunctionName(FunctionID functionId) const
{
    ModuleID module_id;
    ClassID class_id;
    mdTypeDef def_token;

    auto hr = profiler_info_->GetFunctionInfo(functionId, &class_id, &module_id, &def_token);
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

std::optional<std::wstring> NameResolver::ResolveAssemblyName(AssemblyID assemblyId) const
{
    WCHAR name[255];
    ULONG out_name_len;
    AppDomainID app_domain_id;
    ModuleID module_id;
    if (SUCCEEDED(profiler_info_->GetAssemblyInfo(assemblyId, 254, &out_name_len, name, &app_domain_id, &module_id)))
        return std::wstring(name);
    return {};
}

std::optional<std::wstring> NameResolver::ResolveAppDomainName(AppDomainID appDomainId) const
{
    DWORD app_domain_name_count;
    WCHAR app_domain_name[255];
    ProcessID process_id;
    auto hr = profiler_info_->GetAppDomainInfo(appDomainId, 255, &app_domain_name_count, app_domain_name, &process_id);
    if (FAILED(hr))
    {
        return {};
    }

    return std::wstring(app_domain_name);
}

std::optional<std::wstring> NameResolver::ResolveTypeNameByClassIdWithExistingMetaData(
    ClassID classId, IMetaDataImport* pIMDImport) const
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

std::optional<std::wstring> NameResolver::ResolveTypeNameByObjectId(ObjectID objectId) const
{
    ClassID classId;
    auto hr = profiler_info_->GetClassFromObject(objectId, &classId);
    if (SUCCEEDED(hr))
    {
        return this->ResolveTypeNameByObjectIdAndClassId(objectId, classId);
    }
    return {};
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
    if (FAILED(profiler_info_->GetModuleInfo(moduleId, &baseLoadAddress, 1024, &size, moduleName, &assemblyId)))
    {
        return {};
    }
    return std::wstring(moduleName);
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
