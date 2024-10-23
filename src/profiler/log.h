#pragma once

#include <windows.h>
#include <iostream>
#include <chrono>
#include <ostream>
#include <fstream>
#include <mutex>

class Logger
{
protected:
	std::ostream* os_ = nullptr;
    bool include_timestamp_ = true;
public:
    static void initialize(Logger* instance);
    static void DoLog(const std::string& s);
    static void DoLog(const std::wstring& s);
    static void Error(const std::string& s);
    static void Error(const std::wstring& s);
};

class FileLogger : public Logger
{
    std::string filename_;
public:
    explicit FileLogger(std::string s, const bool include_timestamp)
        : filename_(std::move(s))
    {
        os_ = new std::ofstream(filename_.c_str());
        include_timestamp_ = include_timestamp;
    }
    ~FileLogger()
    {
        if (os_)
        {
            const auto of = dynamic_cast<std::ofstream*>(os_);
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

std::string format_iid(REFIID guid);