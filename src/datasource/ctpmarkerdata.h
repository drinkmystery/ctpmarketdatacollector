#ifndef _CTPMARKETDATA_H_
#define _CTPMARKETDATA_H_

#include <memory>
#include <set>
#include <vector>

#include <boost/lockfree/spsc_queue.hpp>

#include "utils/common.h"
#include "utils/structures.h"
#include "datasource/ctpmdspi.h"
#include "datasource/ctptdspi.h"

class CtpMarketData {
public:
    CtpMarketData()  = default;
    ~CtpMarketData() = default;
    int32 init_td(const CtpConfig& ctp_config);
    int32 init_md(const CtpConfig& ctp_config);
    int32 subscribeMarketData(const string& instrument_ids);
    int32 subscribeMarketData(const std::vector<string>& instrument_ids);
    int32 subscribeMarketData();
    bool  getData(MarketData& data);
    bool  empty();
    int32 stop();
    int32 reConnect(const CtpConfig& ctp_config);

private:
    using DataBuffer  = boost::lockfree::spsc_queue<MarketData>;
    using CtpMdApiPtr = std::unique_ptr<CThostFtdcMdApi, std::function<void(CThostFtdcMdApi*)>>;
    using CtpTdApiPtr = std::unique_ptr<CThostFtdcTraderApi, std::function<void(CThostFtdcTraderApi*)>>;

    int32            request_id_ = 0;
    bool             is_inited_  = false;
    std::set<string> inst_ids_;
    DataBuffer       buffer_{8192};

    CtpMdSpi         ctpmdspi_;
    CtpMdApiPtr      ctpmdapi_;

    CtpTdSpi    ctptdspi_;
    CtpTdApiPtr ctptdapi_;
    // 
    std::vector<string>& instrument_id_;

    DISALLOW_COPY_AND_ASSIGN(CtpMarketData);
};

#endif  // _CTPMARKETDATA_H_
