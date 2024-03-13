SET DOTNET_EnableDiagnostics=1
SET COR_ENABLE_PROFILING=1
SET CORECLR_ENABLE_PROFILING=1
SET COR_PROFILER={DC27BF80-3A36-40D4-9278-6415508C4ED6}
SET CORECLR_PROFILER={DC27BF80-3A36-40D4-9278-6415508C4ED6}
SET COR_PROFILER_PATH=.\x64\Debug\OctoProfiler.dll
SET CORECLR_PROFILER_PATH_64=.\x64\Debug\OctoProfiler.dll

IF "%1" == "DEBUG" (
    "C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\windbg.exe" "..\test\HelloProfiler\bin\Debug\net6.0\HelloProfiler.exe"
) ELSE (
    START /WAIT /B ..\test\HelloProfiler\bin\Debug\net6.0\HelloProfiler.exe
)
