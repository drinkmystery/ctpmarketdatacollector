#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <spdlog/spdlog.h>

namespace logger {

inline int32 initLogger() {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
    sinks.emplace_back(std::make_shared<spdlog::sinks::daily_file_sink_st>("logs", 00, 00));
    auto combined_logger = std::make_shared<spdlog::async_logger>("global", begin(sinks), end(sinks), 8192);
    combined_logger->flush_on(spdlog::level::err);
    combined_logger->set_level(spdlog::level::trace);
    combined_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%l][%t]%v");
    spdlog::register_logger(combined_logger);

    return 0;
}

inline int32 releaseLogger() {
    spdlog::drop_all();
    return 0;
}

}  // namespace logger

#define SPDLOG_STR_H(x) #x
#define SPDLOG_STR_HELPER(x) SPDLOG_STR_H(x)
#ifdef _DEBUG
#define DLOG(...) spdlog::get("global")->debug("[" __FILE__ " line #" SPDLOG_STR_HELPER(__LINE__) "]" __VA_ARGS__)
#else
#define DLOG(...)
#endif  // _DEBUG
#define ILOG(...) spdlog::get("global")->info("[" __FILE__ " line #" SPDLOG_STR_HELPER(__LINE__) "]" __VA_ARGS__)
#define WLOG(...) spdlog::get("global")->warn("[" __FILE__ " line #" SPDLOG_STR_HELPER(__LINE__) "]" __VA_ARGS__)
#define ELOG(...) spdlog::get("global")->error("[" __FILE__ " line #" SPDLOG_STR_HELPER(__LINE__) "]" __VA_ARGS__)

#endif  // _LOGGER_H_
