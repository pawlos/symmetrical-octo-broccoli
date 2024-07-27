#include "namedpipe_octo_sink.h"

void namedpipe_octo_sink::send(int command)
{
	//send the data
}


void namedpipe_octo_sink::sync_started()
{
	send(octo_sync_started);
}

void namedpipe_octo_sink::sync_finished()
{
	send(octo_sync_finished);
}

