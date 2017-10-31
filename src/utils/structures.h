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
    string db;
};


struct MarketData {
    // TODO
    string instrument_id;
    string TradingDay;// 日期 格式20170101
	string UpdateTime;// 时间 格式09:16:00
	string ExchangeInstID; // 交易所 对应 CThostFtdcDepthMarketDataField ExchangeID
	double high;
	double close;
	double open;
	double low;
	int volume;

	int BidVolume1;
	int AskVolume1;

    std::chrono::time_point<std::chrono::system_clock> last_tick_time;
    std::chrono::time_point<std::chrono::system_clock> last_update_time;
};

#endif  // _STRUCTURES_H_
