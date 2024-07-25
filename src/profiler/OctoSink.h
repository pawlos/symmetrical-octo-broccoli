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
	virtual void sync_memory_allocated(std::string& allocated_type, long bytes_allocated) = 0;
	virtual void sync_gc_occured(std::string& gc_type) = 0;
	virtual void sync_thread_created(std::string& thread_id) = 0;
	virtual void sync_finished() = 0;
};