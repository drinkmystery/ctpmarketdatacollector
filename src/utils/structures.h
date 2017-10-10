#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#include <chrono>

#include "utils/common.h"

struct CtpConfig {
    string broker_id;
    string user_id;
    string password;
    string md_address;
    string flow_path;
    string instrument_ids;
};

struct MongoConfig {
    string address;
    string user;
    string password;
    string db;
    string collection;
};

struct MarketData {
    string instrument_id;
    string date;
    string value;
    string value1;

    std::chrono::time_point<std::chrono::system_clock> last_tick_time;
    std::chrono::time_point<std::chrono::system_clock> last_update_time;
};

#endif  // _STRUCTURES_H_
