#pragma once

#include <windows.h>
#include <iostream>
#include <chrono>
#include <ostream>
#include <fstream>
#include <mutex>

class Logger {
protected:
	std::ostream* os_;
public:
    static void Initialize(Logger* instance);
    static void DoLog(const std::string& _s);
    static void DoLog(const std::wstring& _s);
    static void Error(const std::string& _s);
    static void Error(const std::wstring& _s);
};

class FileLogger : public Logger
{
    std::string filename_;
public:
    explicit FileLogger(const std::string& _s)
        : filename_(_s)
    {
        os_ = new std::ofstream(filename_.c_str());
    }
    ~FileLogger()
    {
        if (os_)
        {
            std::ofstream* of = dynamic_cast<std::ofstream*>(os_);
            of->close();
            delete os_;
        }
    }
};

class StdOutLogger : public Logger
{
public:
    StdOutLogger()
    {
        os_ = &std::cout;
    }
};

std::string FormatIID(REFIID guid);