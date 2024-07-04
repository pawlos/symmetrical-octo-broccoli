#include "OctoProfiler.h"

HRESULT __stdcall OctoProfiler::QueryInterface(REFIID riid, void** ppvObject)
{
	if (riid == IID_ICorProfilerCallback3 ||
		riid == IID_ICorProfilerCallback2 ||
		riid == IID_ICorProfilerCallback)
	{
		Logger::DoLog(std::format("OctoProfiler::QueryInterface - ProfilerCallback {0}", FormatIID(riid)));
		*ppvObject = this;
		this->AddRef();
		return S_OK;
	}

	*ppvObject = nullptr;
	return E_NOINTERFACE;
}

ULONG __stdcall OctoProfiler::AddRef()
{
	return 1;
}

ULONG __stdcall OctoProfiler::Release()
{
	return 0;
}

HRESULT __stdcall OctoProfiler::Initialize(IUnknown* pICorProfilerInfoUnk)
{
	Logger::DoLog(std::format("OctoProfiler::Initialize started...{0}", this->version_));
	auto hr = pICorProfilerInfoUnk->QueryInterface(IID_ICorProfilerInfo5, reinterpret_cast<void**>(&p_info_));
	if (FAILED(hr))
	{
		return E_FAIL;
	}
	auto versionString = ResolveNetRuntimeVersion();
	Logger::DoLog(std::format(L"OctoProfiler::Detected .NET {}", versionString.value_or(L"<<unknown>>")));
	hr = p_info_->SetEventMask2(COR_PRF_ALL | COR_PRF_MONITOR_ALL | COR_PRF_MONITOR_GC | COR_PRF_ENABLE_STACK_SNAPSHOT | COR_PRF_MONITOR_THREADS, COR_PRF_HIGH_MONITOR_NONE);
	if (FAILED(hr))
	{
		Logger::Error(std::format("OctoProfiler::Initialize - Error setting the event mask. HRESULT: {0:x}", hr));
		return E_FAIL;
	}
	this->name_resolver_ = std::make_unique<NameResolver>(p_info_);
	Logger::DoLog("OctoProfiler::Initialize initialized...");
	return S_OK;
}

std::optional<std::wstring> OctoProfiler::ResolveNetRuntimeVersion() const
{
	USHORT clrRuntimeId{ 0 };
	COR_PRF_RUNTIME_TYPE pRuntimeType{};
	USHORT pMajorVersion{ 0 };
	USHORT pMinorVersion{ 0 };
	USHORT pBuildNumber{ 0 };
	USHORT pQFEVersion{ 0 };
	ULONG pVersionStringLen{ 0 };
	WCHAR versionString[256];
	auto hr = p_info_->GetRuntimeInformation(
		&clrRuntimeId,
		&pRuntimeType,
		&pMajorVersion,
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

HRESULT __stdcall OctoProfiler::Shutdown(void)
{
	std::condition_variable cv;
	std::unique_lock lk(stack_walk_mutex_);
	Logger::DoLog("OctoProfiler::Prepare for shutdown...");
	cv.wait_for(lk, std::chrono::seconds(20));
	Logger::DoLog("OctoProfiler::Shutdown...");
	return S_OK;
}

HRESULT __stdcall OctoProfiler::AppDomainCreationStarted(AppDomainID appDomainId)
{
	auto appDomainName = name_resolver_->ResolveAppDomainName(appDomainId);
	Logger::DoLog(std::format(L"OctoProfiler::App domain creation started: {0}", appDomainName.value_or(L"<<no info>>")));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::AppDomainCreationFinished(AppDomainID appDomainId, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::AppDomainShutdownStarted(AppDomainID appDomainId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::AppDomainShutdownFinished(AppDomainID appDomainId, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::AssemblyLoadStarted(AssemblyID assemblyId)
{
	auto assemblyName = name_resolver_->ResolveAssemblyName(assemblyId);
	Logger::DoLog(std::format(L"OctoProfiler::AssemblyLoadStarted: {0}", assemblyName.value_or(L"<<no info>>")));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus)
{
	auto assemblyName = name_resolver_->ResolveAssemblyName(assemblyId);
	Logger::DoLog(std::format(L"OctoProfiler::AssemblyLoadFinished: {0}", assemblyName.value_or(L"<<no info>>")));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::AssemblyUnloadStarted(AssemblyID assemblyId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::AssemblyUnloadFinished(AssemblyID assemblyId, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ModuleLoadStarted(ModuleID moduleId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ModuleUnloadStarted(ModuleID moduleId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ModuleUnloadFinished(ModuleID moduleId, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ModuleAttachedToAssembly(ModuleID moduleId, AssemblyID AssemblyId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ClassLoadStarted(ClassID classId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ClassLoadFinished(ClassID classId, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ClassUnloadStarted(ClassID classId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ClassUnloadFinished(ClassID classId, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::FunctionUnloadStarted(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock)
{
	auto functionName = name_resolver_->ResolveFunctionName(functionId);

	Logger::DoLog(std::format(L"OctoProfiler::JITCompilationStarted {0}", functionName.value_or(L"<<no info>>")));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::JITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock)
{
	auto functionName = name_resolver_->ResolveFunctionName(functionId);

	Logger::DoLog(std::format(L"OctoProfiler::JITCompilationFinished {0}", functionName.value_or(L"<<no info>>")));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::JITCachedFunctionSearchStarted(FunctionID functionId, BOOL* pbUseCachedFunction)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::JITCachedFunctionSearchFinished(FunctionID functionId, COR_PRF_JIT_CACHE result)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::JITFunctionPitched(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::JITInlining(FunctionID callerId, FunctionID calleeId, BOOL* pfShouldInline)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ThreadCreated(ThreadID threadId)
{
	DWORD win32ThreadId;
	if (FAILED(p_info_->GetThreadInfo(threadId, &win32ThreadId)))
	{
		Logger::Error("Could not resolve thread ID");
		return E_FAIL;
	}

	auto threadName = NameResolver::ResolveCurrentThreadName();
	Logger::DoLog(std::format(L"OctoProfiler::ThreadCreated [{0},{1},{2}]", win32ThreadId, threadId, threadName.value_or(L"<<no info>>")));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::ThreadDestroyed(ThreadID threadId)
{
	Logger::DoLog(std::format("OctoProfiler::ThreadDestroyed {0}", threadId));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadId)
{
	DWORD win32ThreadId;
	if (FAILED(p_info_->GetThreadInfo(managedThreadId, &win32ThreadId)))
	{
		Logger::Error("Could not resolve thread ID");
		return E_FAIL;
	}
	Logger::DoLog(std::format("OctoProfiler::ThreadAssignedToOSThread [{0}]", win32ThreadId));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::RemotingClientInvocationStarted(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RemotingClientSendingMessage(GUID* pCookie, BOOL fIsAsync)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RemotingClientReceivingReply(GUID* pCookie, BOOL fIsAsync)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RemotingClientInvocationFinished(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RemotingServerReceivingMessage(GUID* pCookie, BOOL fIsAsync)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RemotingServerInvocationStarted(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RemotingServerInvocationReturned(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RemotingServerSendingReply(GUID* pCookie, BOOL fIsAsync)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::UnmanagedToManagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ManagedToUnmanagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RuntimeSuspendFinished(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RuntimeSuspendAborted(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RuntimeResumeStarted(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RuntimeResumeFinished(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RuntimeThreadSuspended(ThreadID threadId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RuntimeThreadResumed(ThreadID threadId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::MovedReferences(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], ULONG cObjectIDRangeLength[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall StackSnapshotInfo(FunctionID funcId, UINT_PTR ip, COR_PRF_FRAME_INFO frameInfo, ULONG32 contextSize, BYTE context[], void* clientData)
{
	if (!funcId)
	{
		Logger::DoLog(std::format("OctoProfiler::Native frame {0:x}", ip));
	}
	else
	{
		auto nameResolver = static_cast<NameResolver*>(clientData);
		auto functionName = nameResolver->ResolveFunctionName(funcId);
		Logger::DoLog(std::format(L"OctoProfiler::Managed frame {0} {1:x}", functionName.value_or(L"<<no info>>"), ip));
	}

	return S_OK;
}

HRESULT __stdcall OctoProfiler::ObjectAllocated(ObjectID objectId, ClassID classId)
{
	auto typeName = name_resolver_->ResolveTypeNameByObjectIdAndClassId(objectId, classId);
	SIZE_T bytesAllocated;
	auto hr = p_info_->GetObjectSize2(objectId, &bytesAllocated);
	if (SUCCEEDED(hr))
	{
		Logger::DoLog(std::format(L"OctoProfiler::ObjectAllocated {0} [B] for {1}", bytesAllocated, typeName.value_or(L"<<no info>>")));
		stack_walk_mutex_.lock();
		hr = p_info_->DoStackSnapshot(NULL, &StackSnapshotInfo, COR_PRF_SNAPSHOT_DEFAULT, reinterpret_cast<void *>(name_resolver_.get()), nullptr, 0);
		stack_walk_mutex_.unlock();
		Logger::DoLog("OctoProfiler::DoStackSnapshot end");
		return S_OK;
	}
	return E_FAIL;
}

HRESULT __stdcall OctoProfiler::ObjectsAllocatedByClass(ULONG cClassCount, ClassID classIds[], ULONG cObjects[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ObjectReferences(ObjectID objectId, ClassID classId, ULONG cObjectRefs, ObjectID objectRefIds[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RootReferences(ULONG cRootRefs, ObjectID rootRefIds[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionThrown(ObjectID thrownObjectId)
{
	auto typeName = name_resolver_->ResolveTypeNameByObjectId(thrownObjectId);
	ThreadID threadId;
	auto hr = p_info_->GetCurrentThreadID(&threadId);
	if (FAILED(hr))
	{
		Logger::Error("Could not obtain current thread.");
		return E_FAIL;
	}
	auto threadName = NameResolver::ResolveCurrentThreadName();
	Logger::DoLog(std::format(L"OctoProfiler::ExceptionThrown {0} on thread {1},{2}",
		typeName.value_or(L"<<no info>>"),
		threadId,
		threadName.value_or(L"<<no info>>")));
	stack_walk_mutex_.lock();
	hr = p_info_->DoStackSnapshot(NULL, &StackSnapshotInfo, COR_PRF_SNAPSHOT_DEFAULT, reinterpret_cast<void*>(name_resolver_.get()), nullptr, 0);
	stack_walk_mutex_.unlock();
	Logger::DoLog("OctoProfiler::DoStackSnapshot end");
	return S_OK;
}

HRESULT __stdcall OctoProfiler::ExceptionSearchFunctionEnter(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionSearchFunctionLeave(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionSearchFilterEnter(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionSearchFilterLeave(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionSearchCatcherFound(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionOSHandlerEnter(UINT_PTR __unused)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionOSHandlerLeave(UINT_PTR __unused)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionUnwindFunctionEnter(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionUnwindFunctionLeave(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionUnwindFinallyEnter(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionUnwindFinallyLeave(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionCatcherEnter(FunctionID functionId, ObjectID objectId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionCatcherLeave(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::COMClassicVTableCreated(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable, ULONG cSlots)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::COMClassicVTableDestroyed(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionCLRCatcherFound(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionCLRCatcherExecute(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ThreadNameChanged(ThreadID threadId, ULONG cchName, WCHAR name[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::GarbageCollectionStarted(int cGenerations, BOOL generationCollected[], COR_PRF_GC_REASON reason)
{
	auto gen0 = generationCollected[COR_PRF_GC_GEN_0] ? "GEN0" : "";
	auto gen1 = generationCollected[COR_PRF_GC_GEN_1] ? "GEN1" : "";
	auto gen2 = generationCollected[COR_PRF_GC_GEN_2] ? "GEN2" : "";
	auto genLoh = generationCollected[COR_PRF_GC_LARGE_OBJECT_HEAP] ? "Large Object Heap" : "";
	Logger::DoLog(std::format("OctoProfiler::GarbageCollectionStarted [{0}] [{1}] [{2}] [{3}]", gen0, gen1, gen2, genLoh));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::SurvivingReferences(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], ULONG cObjectIDRangeLength[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::GarbageCollectionFinished(void)
{
	Logger::DoLog("OctoProfiler::GarbageCollectionFinished");
	return S_OK;
}

HRESULT __stdcall OctoProfiler::FinalizeableObjectQueued(DWORD finalizerFlags, ObjectID objectID)
{
	auto typeName = name_resolver_->ResolveTypeNameByObjectId(objectID);
	Logger::DoLog(std::format(L"OctoProfiler::FinalizeableObjectQueued {0}", typeName.value_or(L"<<unknown>>")));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::RootReferences2(ULONG cRootRefs, ObjectID rootRefIds[], COR_PRF_GC_ROOT_KIND rootKinds[], COR_PRF_GC_ROOT_FLAGS rootFlags[], UINT_PTR rootIds[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::HandleCreated(GCHandleID handleId, ObjectID initialObjectId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::HandleDestroyed(GCHandleID handleId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::InitializeForAttach(IUnknown* pCorProfilerInfoUnk, void* pvClientData, UINT cbClientData)
{
	Logger::DoLog("OctoProfiler::InitializeForAttach");
	auto hr = pCorProfilerInfoUnk->QueryInterface(IID_ICorProfilerInfo5, reinterpret_cast<void**>(&p_info_));
	if (FAILED(hr))
	{
		return E_FAIL;
	}
	auto versionString = ResolveNetRuntimeVersion();
	Logger::DoLog(std::format(L"OctoProfiler::Detected .NET {}", versionString.value_or(L"<<unknown>>")));
	hr = p_info_->SetEventMask2(COR_PRF_MONITOR_EXCEPTIONS | COR_PRF_MONITOR_GC, COR_PRF_HIGH_MONITOR_NONE);
	if (FAILED(hr))
	{
		Logger::Error(std::format("OctoProfiler::InitializeForAttach - Error setting the event mask. HRESULT: {0:x}", hr));
		return E_FAIL;
	}
	this->name_resolver_ = std::make_unique<NameResolver>(p_info_);
	Logger::DoLog("OctoProfiler::Initialize initialized...");
	return S_OK;
}

HRESULT __stdcall OctoProfiler::ProfilerAttachComplete()
{
	Logger::DoLog("OctoProfiler::ProfilerAttachComplete");
	return S_OK;
}

HRESULT __stdcall OctoProfiler::ProfilerDetachSucceeded()
{
	Logger::DoLog("OctoProfiler::ProfilerDetachSucceeded");
	return S_OK;
}