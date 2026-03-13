#include "namedpipe_octo_sink.h"
#include <chrono>

namedpipe_octo_sink::namedpipe_octo_sink()
{
	named_pipe_ = CreateNamedPipeW(
		L"\\\\.\\pipe\\octo_sink",
		PIPE_ACCESS_OUTBOUND,
		PIPE_TYPE_BYTE | PIPE_WAIT,
		1, 65536, 0, 0, nullptr);
	ConnectNamedPipe(named_pipe_, nullptr);
}

namedpipe_octo_sink::~namedpipe_octo_sink()
{
	if (named_pipe_ != INVALID_HANDLE_VALUE)
	{
		DisconnectNamedPipe(named_pipe_);
		CloseHandle(named_pipe_);
		named_pipe_ = INVALID_HANDLE_VALUE;
	}
}

void namedpipe_octo_sink::write_bytes(const void* data, DWORD size)
{
	if (named_pipe_ == INVALID_HANDLE_VALUE)
		return;
	DWORD written;
	WriteFile(named_pipe_, data, size, &written, nullptr);
}

void namedpipe_octo_sink::write_uint8(uint8_t value)
{
	write_bytes(&value, 1);
}

void namedpipe_octo_sink::write_uint32(uint32_t value)
{
	write_bytes(&value, 4);
}

void namedpipe_octo_sink::write_uint64(uint64_t value)
{
	write_bytes(&value, 8);
}

void namedpipe_octo_sink::write_string(const std::string& s)
{
	uint32_t len = static_cast<uint32_t>(s.size());
	write_uint32(len);
	if (len > 0)
		write_bytes(s.data(), len);
}

uint64_t namedpipe_octo_sink::get_timestamp_ns()
{
	using namespace std::chrono;
	return static_cast<uint64_t>(
		duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count());
}

void namedpipe_octo_sink::send_frame(uint8_t cmd)
{
	write_uint8(cmd);
	write_uint64(get_timestamp_ns());
}

void namedpipe_octo_sink::sync_started()
{
	// no-op: connection already established in constructor
}

void namedpipe_octo_sink::sync_finished()
{
	std::lock_guard lock(write_mutex_);
	if (named_pipe_ != INVALID_HANDLE_VALUE)
	{
		send_frame(static_cast<uint8_t>(octo_sync_finished));
		DisconnectNamedPipe(named_pipe_);
		CloseHandle(named_pipe_);
		named_pipe_ = INVALID_HANDLE_VALUE;
	}
}

void namedpipe_octo_sink::sync_profiling_start_info(std::string& net_version_detected, uint32_t sample_rate)
{
	std::lock_guard lock(write_mutex_);
	std::string profiler_version = "v0.0.1";
	send_frame(static_cast<uint8_t>(octo_sync_started));
	write_string(profiler_version);
	write_string(net_version_detected);
	write_uint32(sample_rate);
}

void namedpipe_octo_sink::sync_exception_occured(
	std::string& exception,
	std::string& thread_id,
	std::string& thread_name,
	std::vector<std::string>& frames)
{
	std::lock_guard lock(write_mutex_);
	send_frame(static_cast<uint8_t>(octo_exception));
	write_string(exception);
	write_string(thread_id);
	write_string(thread_name);
	uint32_t frame_count = static_cast<uint32_t>(frames.size());
	write_uint32(frame_count);
	for (const auto& frame : frames)
		write_string(frame);
}

void namedpipe_octo_sink::sync_memory_allocated(
	std::string& allocated_type,
	long bytes_allocated,
	std::vector<std::string>& frames)
{
	std::lock_guard lock(write_mutex_);
	send_frame(static_cast<uint8_t>(octo_memory_alloc));
	write_string(allocated_type);
	write_uint64(static_cast<uint64_t>(bytes_allocated));
	uint32_t frame_count = static_cast<uint32_t>(frames.size());
	write_uint32(frame_count);
	for (const auto& frame : frames)
		write_string(frame);
}

void namedpipe_octo_sink::sync_gc_occured(std::string& gc_type)
{
	std::lock_guard lock(write_mutex_);
	send_frame(static_cast<uint8_t>(octo_gc));
	write_string(gc_type);
}

void namedpipe_octo_sink::sync_thread_created(std::string& thread_info)
{
	std::lock_guard lock(write_mutex_);
	send_frame(static_cast<uint8_t>(octo_new_thread));
	write_string(thread_info);
}

void namedpipe_octo_sink::sync_thread_destroyed(std::string& thread_id)
{
	std::lock_guard lock(write_mutex_);
	send_frame(static_cast<uint8_t>(octo_thread_destroyed));
	write_string(thread_id);
}
