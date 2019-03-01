#ifndef _CTPMARKETDATACOLLECTOR_H_
#define _CTPMARKETDATACOLLECTOR_H_

#include <unordered_map>
#include <atomic>
#include <vector>
#include <thread>

#include <json/json.hpp>

#include "utils/common.h"
#include "datastore/mongostore.h"
#include "datasource/ctpmarkerdata.h"
#include "datasource/ctptdspi.h"

class CtpMarketDataCollector {
public:
    CtpMarketDataCollector() = default;
    ~CtpMarketDataCollector();
    int32 loadConfig(int32 argc, char** argv);
    int32 loadJson(const string& json_file);
    int32 createPath();
    int32 init();
    int32 start();
    int32 stop();
    int32 reConnect();
    bool  isRunning() const;

private:
    void loop();
    void process();
    void tryRecord(MarketData& data);

    using json    = nlohmann::json;
    using DataMap = std::unordered_map<string, MarketData>;

    std::atomic<bool>   is_running_  = ATOMIC_FLAG_INIT;
    bool                is_configed_ = false;
    bool                is_inited_   = false;
    MongoConfig         mongo_config_;
    CtpConfig           ctp_config_;
    json                instrument_config_;
    std::vector<string> instrument_ids_;
    DataMap             data_records_;
    MongoStore          mongo_store_;
    std::thread         inter_thread_;
    CtpMarketData       ctp_md_data_;
    Ct

    DISALLOW_COPY_AND_ASSIGN(CtpMarketDataCollector);
};

#endif  // _CTPMARKETDATACOLLECTOR_H_
