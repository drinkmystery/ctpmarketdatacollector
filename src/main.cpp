#include <csignal>

#include <thread>

#include "utils/common.h"
#include "utils/logger.h"
#include "collector/ctpmarketdatacollector.h"

namespace {
volatile std::sig_atomic_t gSignalStatus;
volatile std::sig_atomic_t gIsRunning;
}

extern "C" void signal_handler(int signal) {
    gSignalStatus = signal;
    gIsRunning = 0;
    ILOG("Detect signal:{}", signal);
}

int32 main(int32 argc, char** argv) {
    gIsRunning = 1;
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGSEGV, signal_handler);
    std::signal(SIGINT, signal_handler);
    std::signal(SIGILL, signal_handler);
    std::signal(SIGABRT, signal_handler);
    std::signal(SIGFPE, signal_handler);
    std::signal(SIGBREAK, signal_handler);

    logger::initLogger();
    int32 result = 0;

    CtpMarketDataCollector collector;

    result = collector.loadConfig(argc, argv);
    if (result != 0) {
        ELOG("Collector load config failed! Result:{}", result);
        return -1;
    }
    ILOG("Collector load config success!");

    result = collector.createPath();
    if (result != 0) {
        ELOG("Collector create path failed! Result:{}", result);
        return -2;
    }
    ILOG("Collector create path success!");

    result = collector.init();
    if (result != 0) {
        ELOG("Collector init failed! Result:{}", result);
        return -3;
    }
    ILOG("Collector init success!");

    result = collector.start();
    if (result != 0) {
        ELOG("Collector start failed! Result:{}", result);
        return -4;
    }
    ILOG("Collector start success!");

    while (gIsRunning) {
        std::this_thread::yield();
    }
    result = collector.stop();
    ILOG("Collector is not running, exited! Result:{}", result);

    return result;
}
