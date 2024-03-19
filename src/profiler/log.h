#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <cstdarg>

class Logger {	
public:
	static void DoLog(const char* format, ...);
};
