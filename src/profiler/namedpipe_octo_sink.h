#pragma once
#include "OctoSink.h"
#include "Windows.h"
#include <cstdint>
#include <mutex>

class namedpipe_octo_sink : public octo_sink
{
private:
	HANDLE named_pipe_ = INVALID_HANDLE_VALUE;
	std::mutex write_mutex_;
	void write_bytes(const void* data, DWORD size);
	void write_uint8(uint8_t value);
	void write_uint32(uint32_t value);
	void write_uint64(uint64_t value);
	void write_string(const std::string& s);
	uint64_t get_timestamp_ns();
	void send_frame(uint8_t cmd);
public:
	namedpipe_octo_sink();
	~namedpipe_octo_sink() override;
	void sync_started() override;
	void sync_finished() override;
	void sync_profiling_start_info(std::string& net_version_detected) override;
	void sync_exception_occured(std::string& exception, std::string& thread_id, std::string& thread_name, std::vector<std::string>& frames) override;
	void sync_memory_allocated(std::string& allocated_type, long bytes_allocated, std::vector<std::string>& frames) override;
	void sync_gc_occured(std::string& gc_type) override;
	void sync_thread_created(std::string& thread_info) override;
	void sync_thread_destroyed(std::string& thread_id) override;
};
