#include "log.h"
#include <mutex>

std::mutex m;
void Logger::DoLog(const char* format, ...)
{
	va_list args;

	va_start(args, format);
	auto now = std::chrono::high_resolution_clock::now();
	m.lock();
	std::cout << "[" << now.time_since_epoch() << "] ";
	vprintf(format, args);
	std::cout << std::endl;
	m.unlock();
	va_end(args);
}