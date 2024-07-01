#include "log.h"
#include <mutex>

std::mutex m;
static Logger* g_logger;
void Logger::Initialize(Logger* instance)
{
    g_logger = instance;
}

void Logger::DoLog(const std::string& _s)
{
    auto now = std::chrono::high_resolution_clock::now();
    m.lock();
    (*g_logger->m_os) << "[" << now.time_since_epoch() << "] ";
    (*g_logger->m_os) << _s;
    (*g_logger->m_os) << '\n';
    g_logger->m_os->flush();
    m.unlock();
}

void Logger::DoLog(const std::wstring& _s)
{
    const std::string s(_s.begin(), _s.end());
    Logger::DoLog(s);
}

void Logger::Error(const std::string& _s)
{
    auto now = std::chrono::high_resolution_clock::now();
    std::cerr << "[" << now.time_since_epoch() << "] ";
    std::cerr << "\033[31m" << _s << "\033[0m";
    std::cerr << '\n';
    std::cerr.flush();
}

void Logger::Error(const std::wstring& _s)
{
    const std::string s(_s.begin(), _s.end());
    Logger::DoLog(s);
}

std::string FormatIID(REFIID guid)
{
    return std::format("{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}