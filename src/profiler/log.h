#pragma once

#include <windows.h>
#include <iostream>
#include <chrono>
#include <ostream>
#include <fstream>
#include <mutex>

class Logger {
protected:
	std::ostream* m_os;
public:	
    static void Initialize(Logger* instance);
    static void DoLog(const std::string& _s);
    static void DoLog(const std::wstring& _s);
    static void Error(const std::string& _s);
    static void Error(const std::wstring& _s);
};

class FileLogger : public Logger
{
    std::string m_filename;
public:
    explicit FileLogger(const  std::string& _s)
        : m_filename(_s)
    {
        m_os = new std::ofstream(m_filename.c_str());
    }
    ~FileLogger()
    {
        if (m_os)
        {
            std::ofstream* of = static_cast<std::ofstream*>(m_os);
            of->close();
            delete m_os;
        }
    }
};

class StdOutLogger : public Logger
{
public:
    StdOutLogger()
    {
        m_os = &std::cout;
    }
};