#include "ctpmarketdatacollector.h"

#include <iostream>
#include <chrono>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <date/date.h>

#include "utils/logger.h"

CtpMarketDataCollector::~CtpMarketDataCollector() {
    is_running_.store(false, std::memory_order_release);
    if (inter_thread_.joinable()) {
        inter_thread_.join();
    }
}

int32 CtpMarketDataCollector::loadConfig(int32 argc, char** argv) {
    is_configed_ = false;

    namespace fs = boost::filesystem;
    namespace po = boost::program_options;
    po::variables_map app_options;

    // clang-format off
    po::options_description cmdline_desc("Options");
    cmdline_desc.add_options()
        ("help,h", "print this help message")
        ("conf,c", po::value<string>()->required(), "conf file");
    // clang-format on

    try {
        po::store(po::parse_command_line(argc, argv, cmdline_desc), app_options);
        if (app_options.count("help")) {
            std::cout << cmdline_desc;
            return -1;
        }
        po::notify(app_options);
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cerr << cmdline_desc;
        return -2;
    } catch (...) {
        std::cerr << "Unknown error!\n";
        std::cerr << cmdline_desc;
        return -3;
    }

    // clang-format off
    po::options_description conf_desc("Conf");
    conf_desc.add_options()
        ("ctp.brokerID", po::value<string>()->required())
        ("ctp.userID", po::value<string>()->required())
        ("ctp.password", po::value<string>()->required())
        ("ctp.mdAddress", po::value<string>()->required())
        ("ctp.flowPath", po::value<string>()->required())
        ("ctp.instrumentIDs", po::value<string>()->required())
        ("mongo.address", po::value<string>()->required())
        ("mongo.db", po::value<string>()->required());
    // clang-format on

    try {
        auto conf_file_path = fs::path(app_options["conf"].as<string>());
        if (!fs::exists(conf_file_path) || !fs::is_regular_file(conf_file_path)) {
            std::cerr << "Error: No such conf file: " << conf_file_path;
            return -4;
        }
        po::store(po::parse_config_file<char>(app_options["conf"].as<string>().c_str(), conf_desc), app_options);
        po::notify(app_options);
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cerr << conf_desc;
        return -5;
    } catch (...) {
        std::cerr << "Unknown error!\n";
        std::cerr << conf_desc;
        return -6;
    }

    try {
        ctp_config_.broker_id      = app_options["ctp.brokerID"].as<string>();
        ctp_config_.user_id        = app_options["ctp.userID"].as<string>();
        ctp_config_.password       = app_options["ctp.password"].as<string>();
        ctp_config_.md_address     = app_options["ctp.mdAddress"].as<string>();
        ctp_config_.flow_path      = app_options["ctp.flowPath"].as<string>();
        ctp_config_.instrument_ids = app_options["ctp.instrumentIDs"].as<string>();
        mongo_config_.address      = app_options["mongo.address"].as<string>();
        mongo_config_.db           = app_options["mongo.db"].as<string>();
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return -7;
    } catch (...) {
        std::cerr << "Unknown error!\n";
        return -8;
    }

    is_configed_ = true;
    return 0;
}

int32 CtpMarketDataCollector::createPath() {
    if (!is_configed_) {
        ELOG("Collector is not configured!");
        return -1;
    }

    namespace fs = boost::filesystem;

    try {
        auto flow_path = fs::path(ctp_config_.flow_path);
        if (!fs::exists(flow_path)) {
            if (fs::create_directory(flow_path)) {
                return 0;
            } else {
                ELOG("Flow path create failed!");
                return -1;
            }
        }
        if (!fs::is_directory(flow_path)) {
            ELOG("Flow path is not a directory!");
            return -2;
        }
    } catch (std::exception& e) {
        ELOG("Create path failed!{}", e.what());
        return -3;
    } catch (...) {
        ELOG("Create path failed!");
        return -4;
    }

    return 0;
}

int32 CtpMarketDataCollector::init() {
    if (!is_configed_) {
        ELOG("Collector is not configured!");
        return -1;
    }
    is_inited_ = false;

    int32 init_result = 0;

    try {
        init_result = mongo_store_.init(mongo_config_);
        if (init_result != 0) {
            ELOG("MongoDb init failed! Init result:{}", init_result);
            return init_result;
        }

        init_result = ctp_md_data_.init(ctp_config_);
        if (init_result != 0) {
            ELOG("MarketData init failed! Init result:{}", init_result);
            return init_result;
        }
    } catch (std::exception& e) {
        ELOG("Init failed!{}", e.what());
        return -2;
    } catch (...) {
        ELOG("Init failed!");
        return -3;
    }

    is_inited_ = true;
    return init_result;
}

int32 CtpMarketDataCollector::start() {
    if (is_inited_ == false) {
        ELOG("Collector is not inited!");
        return -1;
    }
    mongo_store_.start();
    is_running_.store(true, std::memory_order_release);
    inter_thread_ = std::thread(&CtpMarketDataCollector::loop, this);
    // ctp_md_data_ is started in CtpMarketDataCollector::init by ctp_md_data_.init
    ctp_md_data_.subscribeMarketData(ctp_config_.instrument_ids);
    return 0;
}

int32 CtpMarketDataCollector::stop() {
    ctp_md_data_.stop();
    DLOG("MarketData stop ok.");
    is_running_.store(false, std::memory_order_release);
    if (inter_thread_.joinable()) {
        inter_thread_.join();
    }
    while (!ctp_md_data_.empty()) {
        process();
    }
    DLOG("Collector stop ok.");
    mongo_store_.stop();
    DLOG("Mongo store stop ok.");
    return 0;
}

int32 CtpMarketDataCollector::reConnect() {
    try {
        auto result = ctp_md_data_.reConnect(ctp_config_);
        if (result != 0) {
            ELOG("MarketData reconnect failed! Result:{}", result);
            return -1;
        }
        ctp_md_data_.subscribeMarketData(ctp_config_.instrument_ids);
        ILOG("MarketData reconnect success!");
    } catch (std::exception& e) {
        ELOG("MarketData reconnect failed! {}", e.what());
        return -2;
    } catch (...) {
        ELOG("MarketData reconnect failed!");
        return -3;
    }
    return 0;
}

bool CtpMarketDataCollector::isRunning() const {
    return is_running_.load(std::memory_order_relaxed);
}

void CtpMarketDataCollector::loop() {
    while (is_running_.load(std::memory_order_relaxed)) {
        process();
        // should yield?
        std::this_thread::yield();
    }
}

void CtpMarketDataCollector::process() {
    while (!ctp_md_data_.empty()) {
        DLOG("Collector process one tick data");
        MarketData tick_data;
        if (!ctp_md_data_.getData(tick_data)) {
            continue;
        }

        auto it = data_records_.find(tick_data.instrument_id);
        if (it != data_records_.end()) {
            auto new_tick_mintues  = date::floor<std::chrono::minutes>(tick_data.last_tick_time);
            auto last_tick_minutes = date::floor<std::chrono::minutes>(it->second.last_tick_time);

            if (new_tick_mintues != last_tick_minutes) {
                // Try record one mintue data into Mongo.
                tryRecord(it->second);
            } else {
                // Memory updtae the highest and lowest inside one mintue.
                tick_data.high = std::max(it->second.high, tick_data.high);
                tick_data.low  = std::min(it->second.low, tick_data.low);
                // Memory update open and volume inside one mintue.
                tick_data.open = it->second.open;
                tick_data.volume += it->second.volume;
            }
            tick_data.last_record_time = it->second.last_record_time;

            it->second = tick_data;
            DLOG("Collector exist Instrument Id:{}", tick_data.instrument_id);
        } else {
            data_records_.insert({tick_data.instrument_id, tick_data});
            DLOG("Collector new Instrument Id:{}", tick_data.instrument_id);
        }
        DLOG("Collector process one tick data ok!");
    }

    for (auto& it : data_records_) {
        tryRecord(it.second);
    }
}

void CtpMarketDataCollector::tryRecord(MarketData& data) {
    auto now_minutes         = date::floor<std::chrono::minutes>(std::chrono::system_clock::now());
    auto last_record_minutes = date::floor<std::chrono::minutes>(data.last_record_time);

    if (last_record_minutes == now_minutes) {
        return;
    }

    data.last_record_time = now_minutes;
    mongo_store_.getBuffer().push(data);
    DLOG("Collector try record one data!");
    data.volume = 0;

    return;
}
