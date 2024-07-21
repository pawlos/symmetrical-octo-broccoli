#include "log.h"
#include <mutex>

std::mutex m;
static Logger* g_logger;
void Logger::initialize(Logger* instance)
{
    g_logger = instance;
}

void Logger::DoLog(const std::string& s)
{
    m.lock();
    if (g_logger->include_timestamp_)
    {
        const auto now = std::chrono::high_resolution_clock::now();
        (*g_logger->os_) << "[" << now.time_since_epoch() << "] ";
    }
    (*g_logger->os_) << s;
    (*g_logger->os_) << '\n';
    g_logger->os_->flush();
    m.unlock();
}

void Logger::DoLog(const std::wstring& s)
{
    const std::string log_string(s.begin(), s.end());
    DoLog(log_string);
}

void Logger::Error(const std::string& s)
{
    if (g_logger->include_timestamp_)
    {
        const auto now = std::chrono::high_resolution_clock::now();
        std::cerr << "[" << now.time_since_epoch() << "] ";
    }
    std::cerr << "\033[31m" << s << "\033[0m";
    std::cerr << '\n';
    std::cerr.flush();
}

void Logger::Error(const std::wstring& s)
{
    const std::string log_string(s.begin(), s.end());
    DoLog(log_string);
}

std::string format_iid(REFIID guid)
{
    return std::format("{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}