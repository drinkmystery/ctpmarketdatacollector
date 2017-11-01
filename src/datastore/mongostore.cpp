#include "datastore/mongostore.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>

#include "utils/logger.h"

mongocxx::instance MongoStore::instance_ = {};

MongoStore::~MongoStore() {
    is_running_.store(false, std::memory_order_release);
    if (inter_thread_.joinable()) {
        inter_thread_.join();
    }
}

int32 MongoStore::init(const MongoConfig& mongo_config) {
    try {
        config_ = mongo_config;
        uri_    = {mongo_config.address};
        client_ = {uri_};
        db_     = client_.database(mongo_config.db);
    } catch (const std::exception& e) {
        ELOG("MongoDb init failed! {}", e.what());
        return -1;
    }

    return 0;
}

int32 MongoStore::start() {
    is_running_.store(true, std::memory_order_release);
    inter_thread_ = std::thread(&MongoStore::loop, this);
    return 0;
}

int32 MongoStore::stop() {
    is_running_.store(false, std::memory_order_release);
    if (inter_thread_.joinable()) {
        inter_thread_.join();
    }
    while (!buffer_.empty()) {
        process();
    }
    return 0;
}

MongoStore::DataBuffer& MongoStore::getBuffer() {
    return buffer_;
}

void MongoStore::loop() {
    while (is_running_.load(std::memory_order_relaxed)) {
        process();
    }
}

void MongoStore::process() {
    auto count = buffer_.read_available();
    if (count == 0) {
        // should yield?
        std::this_thread::yield();
        return;
    }
    DLOG("MongoDb {} data", count);
    MarketData data;
    while (buffer_.pop(data)) {
        DLOG("MongoDb pop data");
        try {
            using bsoncxx::builder::basic::kvp;
            bsoncxx::builder::basic::document builder{};
            builder.append(kvp("id", data.instrument_id));
            builder.append(kvp("date", data.trading_day));
            builder.append(kvp("updateTime", data.update_time));
            builder.append(kvp("exchange", data.exchange_id));
            builder.append(kvp("high", data.high));
            builder.append(kvp("close", (data.close)));
            builder.append(kvp("open", (data.open)));
            builder.append(kvp("low", (data.low)));
            builder.append(kvp("volume", (data.volume)));
            builder.append(kvp("BidVolume1", (data.bid_volume1)));
            builder.append(kvp("AskVolume1", (data.ask_volume1)));
            builder.append(kvp("dateTime", bsoncxx::types::b_date(data.last_record_time)));

            db_[data.destination_id].insert_one(builder.view());
            DLOG("MongoDb record one data ok!");
        } catch (const std::exception& e) {
            ELOG("MongoDb insert failed! {}", e.what());
        }
    }
}
