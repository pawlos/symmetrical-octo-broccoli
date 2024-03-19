#include "log.h"

const std::string GetLogTime()
{
	SYSTEMTIME st;

	GetSystemTime(&st);

	char currentTime[84] = "";

	sprintf_s(currentTime, "%.4d%.2d%.2d%.2d%.2d%.2d%.4d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	return std::string(currentTime);
}

void Logger::DoLog(const char* format, ...)
{
	va_list args;

	va_start(args, format);
	printf("[%s] ", GetLogTime().c_str());
	vprintf(format, args);
	printf("\n");
	va_end(args);
}