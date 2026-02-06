# Agent Guidelines for symmetrical-octo-broccoli

This is a .NET Profiler project (OctoProfiler) with a C++ profiler DLL and C# WPF visualizer (OctoVis).

## Build Commands

This project uses Visual Studio 2022 with MSBuild. All building is done locally via VS 2022.

### C++ Profiler (OctoProfiler)
```bash
# Open solution in VS 2022
src\profiler\OctoProfiler.sln

# Build configurations: Debug|x64, Release|x64
# Output: OctoProfiler.dll (COM-based profiler)
```

### C# Visualizer (OctoVis)
```bash
# Open solution in VS 2022
src\visualizer\OctoVis\OctoVis.sln

# Build with .NET 9 SDK
dotnet build src\visualizer\OctoVis\OctoVis.sln

# Run
src\profiler\run.bat
```

### Running the Profiler
```bash
# Use the run.bat script which sets required environment variables:
# COR_ENABLE_PROFILING, COR_PROFILER, etc.
src\profiler\run.bat <target_program>
```

## Testing

There is no formal test framework. Testing is done via:
- `src\test\HelloProfilerFramework\` - .NET Framework test app
- Run profiler against test apps and verify log output
- OctoVis can parse and visualize the generated log files

## Code Style Guidelines

### C++ Code Style

**Formatting:**
- Use tabs for indentation
- Place opening braces on same line
- Member variables use trailing underscore: `name_`, `profiler_info_`

**Naming Conventions:**
- Classes: PascalCase (`OctoProfiler`, `NameResolver`)
- Methods: PascalCase (`Initialize`, `QueryInterface`)
- Local variables: snake_case (`assembly_id`, `thread_id`)
- Member variables: snake_case with trailing underscore (`version_`)

**Standards:**
- C++20 standard (`stdcpp20`)
- Use `#pragma once` in headers
- Use `std::` for standard library (e.g., `std::unique_ptr`, `std::format`)
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`)
- Use `std::optional` for nullable values
- Use `std::format` for string formatting

**Error Handling:**
- COM methods return `HRESULT`
- Use `SUCCEEDED()` / `FAILED()` macros
- Return `E_NOTIMPL` for unimplemented interface methods
- Use `Logger::DoLog()` and `Logger::Error()` for logging

**Includes:**
- Order: Windows headers, ATL, standard library, project headers
- Use quotes for project headers: `#include "log.h"`
- Use angle brackets for system headers: `#include <windows.h>`

### C# Code Style

**Formatting:**
- Use 4 spaces for indentation
- File-scoped namespaces

**Naming Conventions:**
- Classes/structs: PascalCase
- Methods: PascalCase
- Properties: PascalCase
- Local variables: camelCase
- Private fields: camelCase

**Standards:**
- Target .NET 9
- Enable nullable reference types (`<Nullable>enable</Nullable>`)
- Enable implicit usings (`<ImplicitUsings>enable</ImplicitUsings>`)
- Use `var` for local variables when type is obvious

**Modern C# Features:**
- Use records for data models: `public record TypeEntry(string Name, int Count)`
- Use switch expressions for simple mapping
- Use collection expressions: `[1, 2, 3]` instead of `new[] { 1, 2, 3 }`
- Use target-typed new: `new()` when type is clear
- Use null-coalescing: `value ?? default`

**Error Handling:**
- Use exceptions for exceptional cases
- Return null with nullable types for expected missing data
- Validate early with guard clauses

## Project Structure

```
src/
├── profiler/              # C++ profiler DLL (COM-based)
│   ├── OctoProfiler.*     # Main profiler implementation
│   ├── OctoProfilerEnterLeave.*  # Enter/Leave callback version
│   ├── NameResolver.*     # Symbol resolution
│   └── *.vcxproj          # VS project files
├── visualizer/OctoVis/    # C# WPF visualizer
│   ├── OctoVis/           # Main app
│   ├── Parser/            # Log file parsers
│   └── ViewModel/         # MVVM view models
├── profiler-attach/       # Attach-to-process utility
└── test/                  # Test applications
```

## Important Notes

- This is a Windows-only project requiring VS 2022
- Profiler implements ICorProfilerCallback3 COM interface
- Uses CorGuids.lib for .NET profiling APIs
- ATL is used for COM smart pointers (CComQIPtr)
- Profiler outputs structured log files parsed by OctoVis
- No automated CI/CD - manual build and test
