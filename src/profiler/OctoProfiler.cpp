#include "OctoProfiler.h"

HRESULT __stdcall OctoProfiler::QueryInterface(REFIID riid, void** ppvObject)
{
	static const GUID CLSID_ProfilerCallback2 = { 0x8A8CC829, 0xCCF2, 0x49FE, { 0xBB, 0xAE, 0x0F, 0x02, 0x22, 0x28, 0x07, 0x1A } };
	if (riid == CLSID_ProfilerCallback2)
	{
		printf("OctoProfiler::QueryInterface\n");
		*ppvObject = this;
		return S_OK;
	}
		
	return E_NOTIMPL;
}

ULONG __stdcall OctoProfiler::AddRef(void)
{
	return 1;
}

ULONG __stdcall OctoProfiler::Release(void)
{
	return 0;
}

HRESULT __stdcall OctoProfiler::Initialize(IUnknown* pICorProfilerInfoUnk)
{
	auto hr = pICorProfilerInfoUnk->QueryInterface(IID_ICorProfilerInfo2, (void**)&pInfo);
	if (FAILED(hr)) {
		return E_FAIL;
	}
	hr = pInfo->SetEventMask(COR_PRF_ALL);
	if (FAILED(hr))
	{
		printf("Error setting the event mask.\n");
		return E_FAIL;
	}
	printf("OctoProfiler::Initialize initialized...\n");
	return S_OK;
}

HRESULT __stdcall OctoProfiler::Shutdown(void)
{
	pInfo->Release();
	printf("Total allocated bytes: %ld [B]\n", totalAllocatedBytes);
	printf("OctoProfiler::Shutdown...\n");
	return S_OK;
}

HRESULT __stdcall OctoProfiler::AppDomainCreationStarted(AppDomainID appDomainId)
{
	return E_NOTIMPL;
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
	WCHAR name[255];
	ULONG outNameLen;
	AppDomainID appDomainId;
	ModuleID moduleId;	
	pInfo->GetAssemblyInfo(assemblyId, 254, &outNameLen, name, &appDomainId, &moduleId);
	printf("OctoProfiler::AssemblyLoadStarted: %ls\n", name);
	return S_OK;
}

HRESULT __stdcall OctoProfiler::AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus)
{
	return E_NOTIMPL;
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
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::JITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock)
{
	return E_NOTIMPL;
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
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ThreadDestroyed(ThreadID threadId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadId)
{
	return E_NOTIMPL;
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

HRESULT __stdcall OctoProfiler::ObjectAllocated(ObjectID objectId, ClassID classId)
{
	ULONG bytesAllocated;
	auto hr = pInfo->GetObjectSize(objectId, &bytesAllocated);
	if (SUCCEEDED(hr))
	{
		ModuleID moduleId;
		mdTypeDef defToken;
		totalAllocatedBytes += bytesAllocated;
		hr = pInfo->GetClassIDInfo(classId, &moduleId, &defToken);
		if (SUCCEEDED(hr))
		{
			IMetaDataImport *pIMDImport;
			hr = pInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, (IUnknown**)&pIMDImport);
			if (SUCCEEDED(hr))
			{
				WCHAR typeName[255];
				hr = pIMDImport->GetMethodProps(defToken,
					NULL,
					typeName,
					254,
					0,
					0,
					NULL,
					NULL,
					NULL,
					NULL);
				printf("OctoProfiler::ObjectAllocated %ld [B] for %ls\n", bytesAllocated, typeName);
			}
			else
			{
				printf("OctoProfiler::ObjectAllocated %ld [B]\n", bytesAllocated);
			}
		}
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
	printf("OctoProfiler::ExceptionThrown\n");
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
	printf("OctoProfiler::GarbageCollectionStarted\n");
	return S_OK;
}

HRESULT __stdcall OctoProfiler::SurvivingReferences(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], ULONG cObjectIDRangeLength[])
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::GarbageCollectionFinished(void)
{
	return E_NOTIMPL;
}

HRESULT __stdcall OctoProfiler::FinalizeableObjectQueued(DWORD finalizerFlags, ObjectID objectID)
{
	return E_NOTIMPL;
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
