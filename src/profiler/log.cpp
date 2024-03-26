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
    (*g_logger->m_os) << std::endl;
    g_logger->m_os->flush();
    m.unlock();
}

void Logger::DoLog(const std::wstring& _s)
{
    const std::string s(_s.begin(), _s.end());
    Logger::DoLog(s);
}