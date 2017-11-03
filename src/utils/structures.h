﻿#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#include <chrono>

#include "sfitctp/ThostFtdcUserApiStruct.h"

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
    MarketData()                  = default;
    MarketData(const MarketData&) = default;
    MarketData& operator=(const MarketData&) = default;

    MarketData(const CThostFtdcDepthMarketDataField& origin) {
        instrument_id    = origin.InstrumentID;
        action_day       = "";
        action_time      = "";
        exchange_id      = origin.ExchangeInstID;
        high             = origin.LastPrice;
        close            = origin.LastPrice;
        open             = origin.LastPrice;
        low              = origin.LastPrice;
        volume           = origin.Volume;
        bid_volume1      = origin.BidVolume1;
        ask_volume1      = origin.AskVolume1;
        last_tick_time   = std::chrono::system_clock::now();
        last_record_time = last_tick_time;
		md_trading_day = origin.TradingDay;
		md_update_time = origin.UpdateTime;
    }

    string instrument_id;
	string action_day;
	string action_time;
    string exchange_id;  // 交易所 对应 CThostFtdcDepthMarketDataField ExchangeID
    double high;
    double close;
    double open;
    double low;
    int32  volume;
    int32  bid_volume1;
    int32  ask_volume1;
	string md_trading_day;  // 日期 格式20170101
	string md_update_time;  // 时间 格式09:16:00

    std::chrono::time_point<std::chrono::system_clock> last_tick_time;
    std::chrono::time_point<std::chrono::system_clock> last_record_time;

    string destination_id;
};

#endif  // _STRUCTURES_H_
