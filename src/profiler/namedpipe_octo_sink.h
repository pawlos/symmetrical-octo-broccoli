#pragma once
#include "OctoSink.h"
#include "Windows.h"

class namedpipe_octo_sink : public octo_sink
{
private:
	std::string_view pipe_name_ = "\\\\.\\octo_sink";
	HANDLE named_pipe;
	void send(int command);
public:
	void sync_started() override;
	void sync_finished() override;
};