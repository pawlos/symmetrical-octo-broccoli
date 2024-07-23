#pragma once
#include <string>
#include <vector>

// class that allows sending profiler data to certain sinks like file, pipe, EventLog, elastic etc.
class octo_sink
{
public:
	virtual ~octo_sink() = default;
	virtual void sync_profiling_start_info(std::string& net_version_detected) = 0;
	virtual void sync_exception_occured(std::string& exception, std::vector<std::string>& stack_trace) = 0;
};