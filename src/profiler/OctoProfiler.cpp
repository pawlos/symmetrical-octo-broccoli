#include "OctoProfiler.h"

HRESULT __stdcall OctoProfiler::QueryInterface(REFIID riid, void** ppvObject)
{
	Logger::DoLog(std::format("OctoProfiler::QueryInterface - ProfilerCallback {0}", format_iid(riid)));
	if (riid == IID_ICorProfilerCallback3 ||
		riid == IID_ICorProfilerCallback2 ||
		riid == IID_ICorProfilerCallback)
	{
		*ppvObject = this;
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
	if (!pICorProfilerInfoUnk) 
	{
		Logger::Error("pICorProfilerInfoUnk is null");
		return S_OK;
	}
	auto hr = pICorProfilerInfoUnk->QueryInterface(IID_ICorProfilerInfo5, reinterpret_cast<void**>(&p_info_));
	if (FAILED(hr))
	{
		return E_FAIL;
	}
	this->name_resolver_ = std::make_unique<NameResolver>(p_info_);
	const auto version_string = name_resolver_->ResolveNetRuntimeVersion();
	Logger::DoLog(std::format(L"OctoProfiler::Detected .NET {}", version_string.value_or(L"Error getting .NET information")));
	hr = p_info_->SetEventMask2(COR_PRF_ALL | COR_PRF_MONITOR_ALL | COR_PRF_MONITOR_GC | COR_PRF_ENABLE_STACK_SNAPSHOT | COR_PRF_MONITOR_THREADS, COR_PRF_HIGH_MONITOR_NONE);
	if (FAILED(hr))
	{
		Logger::Error(std::format("OctoProfiler::Initialize - Error setting the event mask. HRESULT: {0:x}", hr));
		return E_FAIL;
	}

	Logger::DoLog("OctoProfiler::Initialize initialized...");
	return S_OK;
}

HRESULT __stdcall OctoProfiler::Shutdown()
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
	const auto app_domain_name = name_resolver_->ResolveAppDomainName(appDomainId);
	Logger::DoLog(std::format(L"OctoProfiler::App domain creation started: {0}", app_domain_name.value_or(L"<<no info>>")));
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

HRESULT __stdcall OctoProfiler::AssemblyLoadStarted(const AssemblyID assembly_id)
{
	const auto assembly_name = name_resolver_->ResolveAssemblyName(assembly_id);
	Logger::DoLog(std::format(L"OctoProfiler::AssemblyLoadStarted: {0}", assembly_name.value_or(L"<<no info>>")));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::AssemblyLoadFinished(const AssemblyID assembly_id, HRESULT hrStatus)
{
	const auto assembly_name = name_resolver_->ResolveAssemblyName(assembly_id);
	Logger::DoLog(std::format(L"OctoProfiler::AssemblyLoadFinished: {0}", assembly_name.value_or(L"<<no info>>")));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::AssemblyUnloadStarted(AssemblyID assembly_id)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::AssemblyUnloadFinished(AssemblyID assembly_id, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ModuleLoadStarted(ModuleID module_id)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ModuleLoadFinished(ModuleID module_id, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ModuleUnloadStarted(ModuleID module_id)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ModuleUnloadFinished(ModuleID module_id, HRESULT hrStatus)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ModuleAttachedToAssembly(ModuleID module_id, AssemblyID assembly_id)
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
	const auto function_name = name_resolver_->ResolveFunctionName(functionId);

	Logger::DoLog(std::format(L"OctoProfiler::JITCompilationStarted {0}", function_name.value_or(L"<<no info>>")));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::JITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock)
{
	const auto function_name = name_resolver_->ResolveFunctionName(functionId);

	Logger::DoLog(std::format(L"OctoProfiler::JITCompilationFinished {0}", function_name.value_or(L"<<no info>>")));
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
	DWORD win32_thread_id;
	if (FAILED(p_info_->GetThreadInfo(threadId, &win32_thread_id)))
	{
		Logger::Error("Could not resolve thread ID");
		return E_FAIL;
	}

	const auto thread_name = NameResolver::ResolveCurrentThreadName();
	Logger::DoLog(std::format(L"OctoProfiler::ThreadCreated [{0},{1},{2}]", win32_thread_id, threadId, thread_name.value_or(L"<<no info>>")));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::ThreadDestroyed(ThreadID threadId)
{
	Logger::DoLog(std::format("OctoProfiler::ThreadDestroyed {0}", threadId));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadId)
{
	DWORD win32_thread_id;
	if (FAILED(p_info_->GetThreadInfo(managedThreadId, &win32_thread_id)))
	{
		Logger::Error("Could not resolve thread ID");
		return E_FAIL;
	}
	Logger::DoLog(std::format("OctoProfiler::ThreadAssignedToOSThread [{0}]", win32_thread_id));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::RemotingClientInvocationStarted()
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

HRESULT __stdcall OctoProfiler::RemotingClientInvocationFinished()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RemotingServerReceivingMessage(GUID* pCookie, BOOL fIsAsync)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RemotingServerInvocationStarted()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RemotingServerInvocationReturned()
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

HRESULT __stdcall OctoProfiler::RuntimeSuspendFinished()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RuntimeSuspendAborted()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RuntimeResumeStarted()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::RuntimeResumeFinished()
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

HRESULT __stdcall StackSnapshotInfo(const FunctionID func_id, UINT_PTR ip, const COR_PRF_FRAME_INFO frame_info, ULONG32 contextSize, BYTE context[], void* clientData)
{
	if (!func_id)
	{
		Logger::DoLog(std::format("OctoProfiler::Native frame {0:x}", ip));
	}
	else
	{
		const auto name_resolver = static_cast<NameResolver*>(clientData);
		const auto function_name = name_resolver->ResolveFunctionNameWithFrameInfo(func_id, frame_info);
		Logger::DoLog(std::format(L"OctoProfiler::Managed frame {0} {1:x}", function_name.value_or(L"<<no info>>"), ip));
	}

	return S_OK;
}

HRESULT __stdcall OctoProfiler::ObjectAllocated(const ObjectID object_id, const ClassID class_id)
{
	const auto type_name = name_resolver_->ResolveTypeNameByObjectIdAndClassId(object_id, class_id);
	SIZE_T bytes_allocated;
	auto hr = p_info_->GetObjectSize2(object_id, &bytes_allocated);
	if (SUCCEEDED(hr))
	{
		Logger::DoLog(std::format(L"OctoProfiler::ObjectAllocated {0} [B] for {1}", bytes_allocated, type_name.value_or(L"<<no info>>")));
		stack_walk_mutex_.lock();
		hr = p_info_->DoStackSnapshot(0, &StackSnapshotInfo, COR_PRF_SNAPSHOT_DEFAULT, name_resolver_.get(), nullptr, 0);
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
	const auto type_name = name_resolver_->ResolveTypeNameByObjectId(thrownObjectId);
	ThreadID thread_id;
	auto hr = p_info_->GetCurrentThreadID(&thread_id);
	if (FAILED(hr))
	{
		Logger::Error("Could not obtain current thread.");
		return E_FAIL;
	}
	const auto thread_name = NameResolver::ResolveCurrentThreadName();
	Logger::DoLog(std::format(L"OctoProfiler::ExceptionThrown {0} on thread {1},{2}",
		type_name.value_or(L"<<no info>>"),
		thread_id,
		thread_name.value_or(L"<<no info>>")));
	stack_walk_mutex_.lock();
	hr = p_info_->DoStackSnapshot(NULL, &StackSnapshotInfo, COR_PRF_SNAPSHOT_DEFAULT, name_resolver_.get(), nullptr, 0);
	stack_walk_mutex_.unlock();
	Logger::DoLog("OctoProfiler::DoStackSnapshot end");
	return S_OK;
}

HRESULT __stdcall OctoProfiler::ExceptionSearchFunctionEnter(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionSearchFunctionLeave()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionSearchFilterEnter(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionSearchFilterLeave()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionSearchCatcherFound(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionOSHandlerEnter(UINT_PTR _unused)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionOSHandlerLeave(UINT_PTR _unused)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionUnwindFunctionEnter(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionUnwindFunctionLeave()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionUnwindFinallyEnter(FunctionID functionId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionUnwindFinallyLeave()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionCatcherEnter(FunctionID functionId, ObjectID objectId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionCatcherLeave()
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

HRESULT __stdcall OctoProfiler::ExceptionCLRCatcherFound()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ExceptionCLRCatcherExecute()
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ThreadNameChanged(ThreadID thread_id, ULONG cch_name, WCHAR name[])
{
	Logger::DoLog(std::format(L"OctoProfiler::ThreadNameChanged {0:X};{1}", thread_id, std::wstring(name)));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::GarbageCollectionStarted(int cGenerations, BOOL generationCollected[], COR_PRF_GC_REASON reason)
{
	auto gen0 = generationCollected[COR_PRF_GC_GEN_0] ? "GEN0" : "";
	auto gen1 = generationCollected[COR_PRF_GC_GEN_1] ? "GEN1" : "";
	auto gen2 = generationCollected[COR_PRF_GC_GEN_2] ? "GEN2" : "";
	auto gen_loh = generationCollected[COR_PRF_GC_LARGE_OBJECT_HEAP] ? "Large Object Heap" : "";
	Logger::DoLog(std::format("OctoProfiler::GarbageCollectionStarted [{0}] [{1}] [{2}] [{3}]", gen0, gen1, gen2, gen_loh));
	return S_OK;
}

HRESULT __stdcall OctoProfiler::SurvivingReferences(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], ULONG cObjectIDRangeLength[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::GarbageCollectionFinished()
{
	Logger::DoLog("OctoProfiler::GarbageCollectionFinished");
	return S_OK;
}

HRESULT __stdcall OctoProfiler::FinalizeableObjectQueued(DWORD finalizer_flags, const ObjectID object_id)
{
	const auto type_name = name_resolver_->ResolveTypeNameByObjectId(object_id);
	Logger::DoLog(std::format(L"OctoProfiler::FinalizeableObjectQueued {0}", type_name.value_or(L"<<unknown>>")));
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
	this->name_resolver_ = std::make_unique<NameResolver>(p_info_);
	const auto version_string = name_resolver_->ResolveNetRuntimeVersion();
	Logger::DoLog(std::format(L"OctoProfiler::Detected .NET {}", version_string.value_or(L"Error getting .NET information")));
	hr = p_info_->SetEventMask2(COR_PRF_MONITOR_EXCEPTIONS | COR_PRF_MONITOR_GC, COR_PRF_HIGH_MONITOR_NONE);
	if (FAILED(hr))
	{
		Logger::Error(std::format("OctoProfiler::InitializeForAttach - Error setting the event mask. HRESULT: {0:x}", hr));
		return E_FAIL;
	}

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