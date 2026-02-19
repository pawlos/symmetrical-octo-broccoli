# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

OctoProfiler is a Windows-only .NET profiler consisting of two components:
- **C++ profiler DLL** (`src/profiler/`) — a COM-based CLR profiler that hooks into a target .NET process and writes structured log files
- **C# WPF visualizer** (`src/visualizer/OctoVis/`) — parses the log files and displays memory/performance data

## Build Commands

Both components require **Visual Studio 2022** on Windows.

### C++ Profiler
Open `src/profiler/OctoProfiler.sln` in VS 2022. Build configurations: `Debug|x64` and `Release|x64`. Output is `OctoProfiler.dll`.

### C# Visualizer
```bash
dotnet build src/visualizer/OctoVis/OctoVis.sln
```

### Running the Profiler Against the Test App
```bash
# From src/profiler/ directory — defaults to memory/event profiling mode
run.bat

# Enable enter/leave (performance/call-trace) mode
run.bat ENTERLEAVE
```

The batch script sets the required environment variables (`COR_ENABLE_PROFILING`, `COR_PROFILER` GUID `{10B46309-C972-4F33-B5AB-5E6E3EBA2B1A}`, `COR_PROFILER_PATH`, etc.) and runs `src/test/HelloProfiler/`.

### Key Environment Variables
| Variable | Purpose |
|---|---|
| `OCTO_PROFILER_FILE` | Output log file path; defaults to stdout if unset |
| `OCTO_MONITOR_ENTERLEAVE` | `true` → use `OctoProfilerEnterLeave` (performance mode); omit for memory/event mode |
| `OCTO_LOGGER_INCLUDE_TS` | `true`/`false` — include nanosecond timestamps in log lines |

## Architecture

### Profiler DLL (C++)

The DLL exports `DllGetClassObject` (in `dllmain.cpp`), which reads environment variables and creates an `OctoProfilerFactory`. The factory instantiates one of two profiler implementations depending on `OCTO_MONITOR_ENTERLEAVE`:

- **`OctoProfiler`** (`v0.0.1`) — implements `ICorProfilerCallback3`; tracks memory allocations (`ObjectAllocated`), exceptions, GC events, and thread lifecycle. Used for memory profiling.
- **`OctoProfilerEnterLeave`** (`v0.0.2`) — also implements `ICorProfilerCallback3`; additionally instruments function enter/leave hooks to trace call timing per thread. Used for performance profiling.

Both use **`NameResolver`** to resolve CLR IDs (function, class, module, assembly, thread) to human-readable names via `ICorProfilerInfo5` and `IMetaDataImport`.

**`octo_sink`** (`OctoSink.h`) is an abstract interface for output destinations. Currently only `namedpipe_octo_sink` is implemented (writes over a named pipe `\\.\octo_sink`); the `Logger` class (`log.h`) writes to a file or stdout independently.

Log lines use the format: `[<nanoseconds>ns] <message>`

### Visualizer (C#/WPF)

`MainWindow` lets the user either open an existing log file or launch and profile a new `.exe` directly (setting the same environment variables programmatically).

**Log parsing** is version-dispatched in `Parser/LogParser.cs`: it reads the first `::Initialize started...` line to detect the profiler version string (`v0.0.1` or `v0.0.2`) and delegates to the matching `IParser` implementation:
- `Parser/v0_0_1/LogParser.cs` → produces `MemoryDataModel` (allocations, GC, exceptions)
- `Parser/v0_0_2/LogParser.cs` → produces `PerformanceDataModel` (enter/exit call tree per thread)

`MainWindow.CreateProfilingWindow` checks the returned `IDataModel` type and opens either `MemoryProfileWindow` or `PerformanceProfileWindow`.

The visualizer uses **LiveCharts2** (SkiaSharp-backed) for charting and follows the **MVVM** pattern (`ViewModel/` directory).

### Testing

No automated test framework. Manual testing: build the profiler DLL, run `run.bat` or `run.bat ENTERLEAVE`, then open the generated `log.txt` in OctoVis to verify output.
Test target apps live in `src/test/` (e.g. `HelloProfiler` targeting .NET 8).

## Code Style

### C++
- C++20 (`stdcpp20`), tabs for indentation, `#pragma once`
- Naming: PascalCase for classes/methods, snake_case for locals, `trailing_underscore_` for members
- Use `std::format`, `std::optional`, smart pointers; COM methods return `HRESULT`
- Log via `Logger::DoLog()` / `Logger::Error()`; unimplemented COM callbacks return `E_NOTIMPL`
- Include order: Windows/ATL headers → standard library → project headers (quotes for project headers)

### C#
- .NET 9, file-scoped namespaces, nullable enabled, implicit usings
- 4 spaces indentation; PascalCase for types/methods/properties, camelCase for locals/fields
- Use `var`, records for data models, switch expressions, collection expressions `[...]`, target-typed `new()`
