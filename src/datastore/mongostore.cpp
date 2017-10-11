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
        uri_ = {mongo_config.address};
        client_ = {uri_};
        db_ = client_.database(mongo_config.db);
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
    std::vector<MarketData> datas;
    datas.reserve(count);
    for (size_t index = 0, read = buffer_.pop(&(datas[0]), count); index < read; ++index) {
        try {
            // TODO add more filed
            using bsoncxx::builder::basic::kvp;
            bsoncxx::builder::basic::document builder{};
            builder.append(kvp("id", datas[index].instrument_id));
            builder.append(kvp("date", bsoncxx::types::b_date(datas[index].last_update_time)));
            builder.append(kvp("value", datas[index].value));

            auto collection = db_.collection(datas[index].instrument_id);
            collection.insert_one(builder.view());
        } catch (const std::exception& e) {
            ELOG("MongoDb insert failed! {}", e.what());
        }
    }
}
