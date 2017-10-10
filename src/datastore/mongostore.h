#ifndef _MONGOSTORE_H_
#define _MONGOSTORE_H_

#include <atomic>
#include <thread>

#include <boost/lockfree/spsc_queue.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include "utils/common.h"
#include "utils/structures.h"

class MongoStore {
public:
    using DataBuffer = boost::lockfree::spsc_queue<MarketData>;

    MongoStore() = default;
    ~MongoStore();
    int32 init(const MongoConfig& mongo_config);
    int32 start();
    int32 stop();
    DataBuffer& getBuffer();

private:
    void loop();
    void process();

    static mongocxx::instance instance_;

    std::atomic<bool>    is_running_{ATOMIC_FLAG_INIT};
    MongoConfig          config_;
    mongocxx::uri        uri_;
    mongocxx::client     client_;
    mongocxx::database   db_;
    DataBuffer           buffer_{8192};
    std::thread          inter_thread_;

    DISALLOW_COPY_AND_ASSIGN(MongoStore);
};

#endif  // _MONGOSTORE_H_
