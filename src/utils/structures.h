#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#include <chrono>

#include "sfitctp/ThostFtdcUserApiStruct.h"

#include "utils/common.h"

struct CtpConfig {
    string broker_id;
    string user_id;
    string password;
    string md_address;
    string td_address;
    string td_flow_path;
    string md_flow_path;
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
        exchange_id      = origin.ExchangeID;
        high             = origin.LastPrice;
        close            = origin.LastPrice;
        open             = origin.LastPrice;
        low              = origin.LastPrice;
        volume           = origin.Volume;
        bid_volume1      = origin.BidVolume1;
        ask_volume1      = origin.AskVolume1;
        last_tick_time   = std::chrono::system_clock::now();
        last_record_time = last_tick_time;
        md_trading_day   = origin.TradingDay;
        md_update_time   = origin.UpdateTime;

        highest_price      = origin.HighestPrice;  // 当日最高价
        lowest_price       = origin.LowestPrice;   // 当日最低价
        open_price         = origin.OpenPrice;     //
        PreSettlementPrice = origin.PreSettlementPrice;
        PreClosePrice      = origin.PreClosePrice;
        Turnover           = origin.Turnover;         // 成交金额
        PreOpenInterest    = origin.PreOpenInterest;  // 昨日持仓量
        OpenInterest       = origin.OpenInterest;     // 持仓量
        UpperLimitPrice    = origin.UpperLimitPrice;  // 涨停板价格
        LowerLimitPrice    = origin.LowerLimitPrice;  // 跌停板价格
        marketVol          = 0;
    }

    string instrument_id;
    string action_day;
    string action_time;
    string exchange_id;  // 交易所 对应 CThostFtdcDepthMarketDataField ExchangeID
    double high;
    double close;
    double open;
    double low;
    int32  volume;     //累计成交量
    int32  marketVol;  //单位内总成交量
    int32  bid_volume1;
    int32  ask_volume1;
    string md_trading_day;  // 日期 格式20170101
    string md_update_time;  // 时间 格式09:16:00

    double highest_price;  // 当日最高价
    double lowest_price;   // 当日最低价
    double open_price;     //
    double PreSettlementPrice;
    double PreClosePrice;
    double Turnover;         // 成交金额
    double PreOpenInterest;  // 昨日持仓量
    double OpenInterest;     // 持仓量
    double UpperLimitPrice;  // 涨停板价格
    double LowerLimitPrice;  // 跌停板价格

    std::chrono::time_point<std::chrono::system_clock> last_tick_time;
    std::chrono::time_point<std::chrono::system_clock> last_record_time;

    string destination_id;
};
enum TIME_MODE {TICK = 0, MIN_1 = 1};
#endif  // _STRUCTURES_H_
