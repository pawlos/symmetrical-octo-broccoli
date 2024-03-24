#pragma once

#include <windows.h>
#include <iostream>
#include <cstdarg>
#include <chrono>
#include <iostream>

class Logger {	
public:
	static void DoLog(const char* format, ...);
};
