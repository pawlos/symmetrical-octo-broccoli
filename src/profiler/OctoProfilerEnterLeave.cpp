#include "OctoProfilerEnterLeave.h"

EXTERN_C_START

void FuncEnterCallback(
	FunctionID funId,
	UINT_PTR clientData,
	COR_PRF_FRAME_INFO frameInfo,
	COR_PRF_FUNCTION_ARGUMENT_INFO * argInfo);

void FuncLeaveCallback(
	FunctionID funId,
	UINT_PTR clientData,
	COR_PRF_FRAME_INFO frameInfo,
	COR_PRF_FUNCTION_ARGUMENT_INFO* argInfo);

void FuncEnterStub(FunctionID funcId, UINT_PTR clientData)
{			
	if (clientData != NULL)
	{
		auto str = reinterpret_cast<wchar_t*>(clientData);
		Logger::DoLog(std::format(L"OctoProfilerEnterLeave::Enter {0}", str));		
	}
	else
	{
		Logger::DoLog(std::format("OctoProfilerEnterLeave::Enter {0:x}", funcId));
	}
}

void FuncLeaveStub(FunctionID funcId, UINT_PTR clientData)
{
	if (clientData != NULL)
	{
		auto str = reinterpret_cast<wchar_t*>(clientData);
		Logger::DoLog(std::format(L"OctoProfilerEnterLeave::Exit {0}", str));
	}
	else
	{
		Logger::DoLog(std::format("OctoProfilerEnterLeave::Exit {0:x}", funcId));
	}
}

EXTERN_C_END

HRESULT __stdcall OctoProfilerEnterLeave::QueryInterface(REFIID riid, void** ppvObject)
{
	if (riid == IID_ICorProfilerCallback3 ||
		riid == IID_ICorProfilerCallback2 ||
		riid == IID_ICorProfilerCallback)
	{
		Logger::DoLog(std::format("OctoProfilerEnterLeave::QueryInterface - ProfilerCallback {0:8x}", riid.Data1));
		*ppvObject = this;
		this->AddRef();
		return S_OK;
	}

	*ppvObject = nullptr;
	return E_NOINTERFACE;
}

ULONG __stdcall OctoProfilerEnterLeave::AddRef(void)
{
	return 1;
}

ULONG __stdcall OctoProfilerEnterLeave::Release(void)
{
	return 0;
}

UINT_PTR __stdcall MapFunctionId(FunctionID funcId, void *clientData, BOOL* pbHookFunction)
{	
	auto nameResolver = reinterpret_cast<NameResolver*>(clientData);
	auto functionName = nameResolver->ResolveFunctionName(funcId);
	*pbHookFunction = false;

	if (functionName.has_value())
	{
		auto c_ptr = new std::wstring(functionName.value_or(L"<<unknown>>"));
		*pbHookFunction = true;
		return reinterpret_cast<UINT_PTR>(c_ptr->c_str());
	}
	return NULL;
}

HRESULT __stdcall OctoProfilerEnterLeave::Initialize(IUnknown* pICorProfilerInfoUnk)
{
	Logger::DoLog(std::format("OctoProfilerEnterLeave::Initialize started...{0}", this->version));
	auto hr = pICorProfilerInfoUnk->QueryInterface(IID_ICorProfilerInfo5, reinterpret_cast<void**>(&pInfo));
	if (FAILED(hr))
	{
		return E_FAIL;
	}
	auto versionString = ResolveNetRuntimeVersion();
	Logger::DoLog(std::format(L"OctoProfilerEnterLeave::Detected .NET {}", versionString.value_or(L"<<unknown>>")));
	hr = pInfo->SetEventMask2(COR_PRF_MONITOR_ENTERLEAVE, COR_PRF_HIGH_MONITOR_NONE);
	if (FAILED(hr))
	{
		Logger::Error(std::format("OctoProfilerEnterLeave::Initialize - Error setting the event mask. HRESULT: {0:x}", hr));
		return E_FAIL;
	}
	this->nameResolver = std::shared_ptr<NameResolver>(new NameResolver(pInfo));
	this->pInfo->SetFunctionIDMapper2(&MapFunctionId, reinterpret_cast<void*>(nameResolver.get()));
	this->pInfo->SetEnterLeaveFunctionHooks2(
		reinterpret_cast<FunctionEnter2*>(FuncEnterCallback),
		reinterpret_cast<FunctionLeave2*>(FuncLeaveCallback), nullptr);
	Logger::DoLog("OctoProfilerEnterLeave::Initialize initialized...");
	return S_OK;
}

std::optional<std::wstring> OctoProfilerEnterLeave::ResolveNetRuntimeVersion() const
{
	USHORT clrRuntimeId{ 0 };
	COR_PRF_RUNTIME_TYPE pRuntimeType{};
	USHORT pMajorVerion{ 0 };
	USHORT pMinorVersion{ 0 };
	USHORT pBuildNumber{ 0 };
	USHORT pQFEVersion{ 0 };
	ULONG pVersionStringLen{ 0 };
	WCHAR versionString[256];
	auto hr = pInfo->GetRuntimeInformation(
		&clrRuntimeId,
		&pRuntimeType,
		&pMajorVerion,
		&pMinorVersion,
		&pBuildNumber,
		&pQFEVersion,
		255,
		&pVersionStringLen,
		versionString);
	if (FAILED(hr))
	{
		Logger::Error(std::format("OctoProfiler::ResolveNetRuntimeVersion - Error getting .NET information. HRESULT: {0:x}", hr));
		return {};
	}

	return std::wstring(versionString);
}

HRESULT __stdcall OctoProfilerEnterLeave::Shutdown(void)
{
	Logger::DoLog("OctoProfilerEnterLeave::Prepare for shutdown...");
	Logger::DoLog("OctoProfilerEnterLeave::Shutdown...");
	return S_OK;
}

HRESULT __stdcall OctoProfilerEnterLeave::AppDomainCreationStarted(AppDomainID appDomainId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::AppDomainCreationFinished(AppDomainID appDomainId, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::AppDomainShutdownStarted(AppDomainID appDomainId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::AppDomainShutdownFinished(AppDomainID appDomainId, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::AssemblyLoadStarted(AssemblyID assemblyId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::AssemblyUnloadStarted(AssemblyID assemblyId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::AssemblyUnloadFinished(AssemblyID assemblyId, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ModuleLoadStarted(ModuleID moduleId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ModuleUnloadStarted(ModuleID moduleId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ModuleUnloadFinished(ModuleID moduleId, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ModuleAttachedToAssembly(ModuleID moduleId, AssemblyID AssemblyId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ClassLoadStarted(ClassID classId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ClassLoadFinished(ClassID classId, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ClassUnloadStarted(ClassID classId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ClassUnloadFinished(ClassID classId, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::FunctionUnloadStarted(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::JITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::JITCachedFunctionSearchStarted(FunctionID functionId, BOOL* pbUseCachedFunction)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::JITCachedFunctionSearchFinished(FunctionID functionId, COR_PRF_JIT_CACHE result)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::JITFunctionPitched(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::JITInlining(FunctionID callerId, FunctionID calleeId, BOOL* pfShouldInline)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ThreadCreated(ThreadID threadId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ThreadDestroyed(ThreadID threadId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RemotingClientInvocationStarted(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RemotingClientSendingMessage(GUID* pCookie, BOOL fIsAsync)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RemotingClientReceivingReply(GUID* pCookie, BOOL fIsAsync)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RemotingClientInvocationFinished(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RemotingServerReceivingMessage(GUID* pCookie, BOOL fIsAsync)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RemotingServerInvocationStarted(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RemotingServerInvocationReturned(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RemotingServerSendingReply(GUID* pCookie, BOOL fIsAsync)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::UnmanagedToManagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ManagedToUnmanagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RuntimeSuspendFinished(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RuntimeSuspendAborted(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RuntimeResumeStarted(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RuntimeResumeFinished(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RuntimeThreadSuspended(ThreadID threadId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RuntimeThreadResumed(ThreadID threadId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::MovedReferences(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], ULONG cObjectIDRangeLength[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ObjectAllocated(ObjectID objectId, ClassID classId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ObjectsAllocatedByClass(ULONG cClassCount, ClassID classIds[], ULONG cObjects[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ObjectReferences(ObjectID objectId, ClassID classId, ULONG cObjectRefs, ObjectID objectRefIds[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RootReferences(ULONG cRootRefs, ObjectID rootRefIds[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionThrown(ObjectID thrownObjectId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionSearchFunctionEnter(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionSearchFunctionLeave(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionSearchFilterEnter(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionSearchFilterLeave(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionSearchCatcherFound(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionOSHandlerEnter(UINT_PTR __unused)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionOSHandlerLeave(UINT_PTR __unused)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionUnwindFunctionEnter(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionUnwindFunctionLeave(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionUnwindFinallyEnter(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionUnwindFinallyLeave(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionCatcherEnter(FunctionID functionId, ObjectID objectId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionCatcherLeave(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::COMClassicVTableCreated(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable, ULONG cSlots)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::COMClassicVTableDestroyed(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionCLRCatcherFound(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionCLRCatcherExecute(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ThreadNameChanged(ThreadID threadId, ULONG cchName, WCHAR name[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::GarbageCollectionStarted(int cGenerations, BOOL generationCollected[], COR_PRF_GC_REASON reason)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::SurvivingReferences(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], ULONG cObjectIDRangeLength[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::GarbageCollectionFinished(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::FinalizeableObjectQueued(DWORD finalizerFlags, ObjectID objectID)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RootReferences2(ULONG cRootRefs, ObjectID rootRefIds[], COR_PRF_GC_ROOT_KIND rootKinds[], COR_PRF_GC_ROOT_FLAGS rootFlags[], UINT_PTR rootIds[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::HandleCreated(GCHandleID handleId, ObjectID initialObjectId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::HandleDestroyed(GCHandleID handleId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::InitializeForAttach(IUnknown* pCorProfilerInfoUnk, void* pvClientData, UINT cbClientData)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ProfilerAttachComplete(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ProfilerDetachSucceeded(void)
{
	return E_NOTIMPL;
}
