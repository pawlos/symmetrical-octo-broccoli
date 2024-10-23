#include "OctoProfilerEnterLeave.h"
#include <map>
std::map<FunctionID, std::wstring> functionNameDict;
void FuncEnter(FunctionID funcId, UINT_PTR clientData, COR_PRF_FRAME_INFO frameInfo, COR_PRF_FUNCTION_ARGUMENT_INFO *argInfo)
{
	if (clientData != NULL)
	{
		if (!functionNameDict.contains(funcId))
		{
			const auto name_resolver = reinterpret_cast<NameResolver*>(clientData);
			const auto str = name_resolver->ResolveFunctionNameWithFrameInfo(funcId, frameInfo);
			functionNameDict.emplace(funcId, str.value_or(L"<empty>"));
		}
		Logger::DoLog(std::format(L"OctoProfilerEnterLeave::Enter {0}", functionNameDict[funcId]));
	}
	else
	{
		Logger::DoLog(std::format("OctoProfilerEnterLeave::Enter {0:x}", funcId));
	}
}

void FuncLeave(FunctionID funcId, UINT_PTR clientData, COR_PRF_FRAME_INFO frameInfo, COR_PRF_FUNCTION_ARGUMENT_RANGE* argInfo)
{
	if (clientData != NULL)
	{
		std::wstring str = functionNameDict[funcId];
		Logger::DoLog(std::format(L"OctoProfilerEnterLeave::Exit {0}", str));
	}
	else
	{
		Logger::DoLog(std::format("OctoProfilerEnterLeave::Exit {0:x}", funcId));
	}
}

void FuncTail(FunctionID funcId, UINT_PTR clientData, COR_PRF_FRAME_INFO frameInfo)
{
	if (clientData != NULL)
	{
		const auto name_resolver = reinterpret_cast<NameResolver*>(clientData);
		const auto str = name_resolver->ResolveFunctionNameWithFrameInfo(funcId, frameInfo);
		Logger::DoLog(std::format(L"OctoProfilerEnterLeave::Enter(tail) {0}", str.value_or(L"<empty>")));
	}
	else
	{
		Logger::DoLog(std::format("OctoProfilerEnterLeave::Enter(tail) {0:x}", funcId));
	}
}

HRESULT __stdcall OctoProfilerEnterLeave::QueryInterface(REFIID riid, void** ppvObject)
{
	Logger::DoLog(std::format("OctoProfilerEnterLeave::QueryInterface - ProfilerCallback {0}", format_iid(riid)));
	if (riid == IID_ICorProfilerCallback3 ||
		riid == IID_ICorProfilerCallback2 ||
		riid == IID_ICorProfilerCallback)
	{
		*ppvObject = this;
		this->AddRef();
		return S_OK;
	}

	*ppvObject = nullptr;
	return E_NOINTERFACE;
}

ULONG __stdcall OctoProfilerEnterLeave::AddRef()
{
	return 1;
}

ULONG __stdcall OctoProfilerEnterLeave::Release()
{
	return 0;
}

UINT_PTR __stdcall MapFunctionId(FunctionID funcId, void *clientData, BOOL* pbHookFunction)
{
	*pbHookFunction = true;
	return reinterpret_cast<UINT_PTR>(clientData);
}

HRESULT __stdcall OctoProfilerEnterLeave::Initialize(IUnknown* pICorProfilerInfoUnk)
{
	Logger::DoLog(std::format("OctoProfilerEnterLeave::Initialize started...{0}", this->version_));
	auto hr = pICorProfilerInfoUnk->QueryInterface(IID_ICorProfilerInfo5, reinterpret_cast<void**>(&profiler_info_));
	if (FAILED(hr))
	{
		return E_FAIL;
	}
	this->name_resolver_ = std::make_shared<NameResolver>(profiler_info_);
	const auto version_string = name_resolver_->ResolveNetRuntimeVersion();
	Logger::DoLog(std::format(L"OctoProfilerEnterLeave::Detected .NET {}", version_string.value_or(L"Error getting .NET information")));
	hr = profiler_info_->SetEventMask2(COR_PRF_MONITOR_ENTERLEAVE | COR_PRF_ENABLE_FRAME_INFO, COR_PRF_HIGH_MONITOR_NONE);
	if (FAILED(hr))
	{
		Logger::Error(std::format("OctoProfilerEnterLeave::Initialize - Error setting the event mask. HRESULT: {0:x}", hr));
		return E_FAIL;
	}

	this->profiler_info_->SetFunctionIDMapper2(&MapFunctionId, name_resolver_.get());
	this->profiler_info_->SetEnterLeaveFunctionHooks2(
		FuncEnter,
		FuncLeave,
		FuncTail);
	Logger::DoLog("OctoProfilerEnterLeave::Initialize initialized...");
	return S_OK;
}

HRESULT __stdcall OctoProfilerEnterLeave::Shutdown()
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

HRESULT __stdcall OctoProfilerEnterLeave::RemotingClientInvocationStarted()
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

HRESULT __stdcall OctoProfilerEnterLeave::RemotingClientInvocationFinished()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RemotingServerReceivingMessage(GUID* pCookie, BOOL fIsAsync)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RemotingServerInvocationStarted()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RemotingServerInvocationReturned()
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

HRESULT __stdcall OctoProfilerEnterLeave::RuntimeSuspendFinished()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RuntimeSuspendAborted()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RuntimeResumeStarted()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::RuntimeResumeFinished()
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

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionSearchFunctionLeave()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionSearchFilterEnter(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionSearchFilterLeave()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionSearchCatcherFound(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionOSHandlerEnter(UINT_PTR _unused)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionOSHandlerLeave(UINT_PTR _unused)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionUnwindFunctionEnter(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionUnwindFunctionLeave()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionUnwindFinallyEnter(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionUnwindFinallyLeave()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionCatcherEnter(FunctionID functionId, ObjectID objectId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionCatcherLeave()
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

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionCLRCatcherFound()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ExceptionCLRCatcherExecute()
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

HRESULT __stdcall OctoProfilerEnterLeave::GarbageCollectionFinished()
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

HRESULT __stdcall OctoProfilerEnterLeave::ProfilerAttachComplete()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfilerEnterLeave::ProfilerDetachSucceeded()
{
	return E_NOTIMPL;
}
