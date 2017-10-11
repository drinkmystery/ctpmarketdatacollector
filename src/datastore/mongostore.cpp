#include "datastore/mongostore.h"

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
            auto collection = db_.collection(datas[index].instrument_id);
            // clang-format off
            auto doc_value = bsoncxx::builder::stream::document{}
                << "id" << datas[index].instrument_id
                << "date" << datas[index].date
                << "value" << datas[index].value
                << bsoncxx::builder::stream::finalize;
            // clang-format on
            collection.insert_one(doc_value.view());
        } catch (const std::exception& e) {
            ELOG("MongoDb insert failed! {}", e.what());
        }
    }
}
