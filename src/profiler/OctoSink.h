#pragma once
#include <string>
#include <vector>

constexpr int octo_sync_started = 0x00;
constexpr int octo_exception = 0x01;
constexpr int octo_memory_alloc = 0x02;
constexpr int octo_gc = 0x03;
constexpr int octo_new_thread = 0x04;
constexpr int octo_thread_destroyed = 0x05;
constexpr int octo_sync_finished = 0xFF;
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
	virtual void sync_started() = 0;
	virtual void sync_finished() = 0;
};