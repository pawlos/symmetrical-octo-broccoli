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

class NullLogger : public Logger
{
    struct NullBuf : public std::streambuf {
        int overflow(int c) override { return c; }
    } null_buf_;
    std::ostream null_stream_;
public:
    NullLogger() : null_stream_(&null_buf_)
    {
        os_ = &null_stream_;
        include_timestamp_ = false;
    }
};

std::string format_iid(REFIID guid);