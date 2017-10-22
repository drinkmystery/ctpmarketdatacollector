#include <csignal>

#include <thread>

#include "utils/common.h"
#include "utils/logger.h"
#include "utils/global.h"
#include "collector/ctpmarketdatacollector.h"

namespace {
volatile std::sig_atomic_t gSignalStatus;
volatile std::sig_atomic_t gIsRunning;
}

extern "C" void signal_handler(int signal) {
    ILOG("Detect signal:{}", signal);
    gSignalStatus = signal;
    gIsRunning    = 0;
}

int32 main(int32 argc, char** argv) {
    gIsRunning = 1;
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler);

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
        if (global::need_reconnect.load(std::memory_order_relaxed)) {
            auto result = collector.reConnect();
            if (result == 0) {
                ILOG("Collector reconnet success!");
                global::need_reconnect.store(true, std::memory_order_release);
            }
        }
        std::this_thread::yield();
    }
    ILOG("Collector try stop");
    result = collector.stop();
    ILOG("Collector is not running, exited! Result:{}", result);

    return result;
}
