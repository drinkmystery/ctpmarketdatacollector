#include "ctpmdspi.h"

#include "utils/logger.h"

void CtpMdSpi::setOnFrontConnected(std::function<void()>&& fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_connected_fun_ = fun;
}

void CtpMdSpi::setOnFrontDisConnected(std::function<void(int32)>&& fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_disconnected_fun_ = fun;
}

void CtpMdSpi::setOnLoginFun(std::function<void(CThostFtdcRspUserLoginField*, CThostFtdcRspInfoField*)> fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_login_fun_ = fun;
}

void CtpMdSpi::setOnErrorFun(std::function<void(CThostFtdcRspInfoField*)> fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_error_fun_ = fun;
}

void CtpMdSpi::setOnSubFun(std::function<void(CThostFtdcSpecificInstrumentField*, CThostFtdcRspInfoField*)> fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_sub_fun_ = fun;
}

void CtpMdSpi::setOnUnSubFun(std::function<void(CThostFtdcSpecificInstrumentField*, CThostFtdcRspInfoField*)> fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_unsub_fun_ = fun;
}

void CtpMdSpi::setOnDataFun(std::function<void(CThostFtdcDepthMarketDataField*)> fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_data_fun_ = fun;
}

void CtpMdSpi::setOnQuoteFun(std::function<void(CThostFtdcForQuoteRspField*)> fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_quote_fun_ = fun;
}

void CtpMdSpi::OnFrontConnected() {
    ILOG("Ctp connected to front.");
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_connected_fun_) {
        std::invoke(on_connected_fun_);
    }
}

void CtpMdSpi::OnFrontDisconnected(int32 nReason) {
    ILOG("Ctp disconnect from front! Reason:{}", nReason);
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_disconnected_fun_) {
        std::invoke(on_disconnected_fun_, nReason);
    }
}

void CtpMdSpi::OnHeartBeatWarning(int32 nTimeLapse) {
    WLOG("Ctp {}s no heartbeat!", nTimeLapse);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField*      pRspInfo,
                              int32                        nRequestID,
                              bool                         bIsLast) {
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        ELOG("Ctp Login failed! RequestID:{},IsLast:{},ErrorID:{},ErrorMsg:{}",
             nRequestID,
             bIsLast,
             pRspInfo->ErrorID,
             pRspInfo->ErrorMsg);
    } else if (pRspUserLogin && bIsLast) {
        ILOG("Ctp Login success! RequestID:{},IsLast:{},ErrorId:{}", nRequestID, bIsLast,pRspInfo->ErrorID);
    }

    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_login_fun_) {
        std::invoke(on_login_fun_, pRspUserLogin, pRspInfo);
    }
}

void CtpMdSpi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
                               CThostFtdcRspInfoField*    pRspInfo,
                               int32                      nRequestID,
                               bool                       bIsLast) {
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        ELOG("Ctp Logout failed! RequestID:{},IsLast:{},ErrorID:{},ErrorMsg:{}",
             nRequestID,
             bIsLast,
             pRspInfo->ErrorID,
             pRspInfo->ErrorMsg);
    } else if (pUserLogout && bIsLast) {
        ILOG("Ctp Logout success! RequestID:{},IsLast:{}", nRequestID, bIsLast);
    }
}

void CtpMdSpi::OnRspError(CThostFtdcRspInfoField* pRspInfo, int32 nRequestID, bool bIsLast) {
    ELOG("Ctp error happpend RequestID:{},IsLast:{},ErrorID:{},ErrorMsg:{}",
         nRequestID,
         bIsLast,
         pRspInfo->ErrorID,
         pRspInfo->ErrorMsg);
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_error_fun_) {
        std::invoke(on_error_fun_, pRspInfo);
    }
}

void CtpMdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument,
                                  CThostFtdcRspInfoField*            pRspInfo,
                                  int32                              nRequestID,
                                  bool                               bIsLast) {
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        ELOG("Ctp SubMarketData failed! RequestID:{},IsLast:{},ErrorID:{},ErrorMsg:{}",
             nRequestID,
             bIsLast,
             pRspInfo->ErrorID,
             pRspInfo->ErrorMsg);
    } else if (pSpecificInstrument) {
        ILOG("Ctp SubMarketData success! RequestID:{},IsLast:{},InstrumentID:{},ErrodId:{}",
             nRequestID,
             bIsLast,
             pSpecificInstrument->InstrumentID,pRspInfo->ErrorID);
    }
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_sub_fun_) {
        std::invoke(on_sub_fun_, pSpecificInstrument, pRspInfo);
    }
}

void CtpMdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument,
                                    CThostFtdcRspInfoField*            pRspInfo,
                                    int32                              nRequestID,
                                    bool                               bIsLast) {
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        ELOG("Ctp UnSubMarketData failed! RequestID:{},IsLast:{},ErrorID:{},ErrorMsg:{}",
             nRequestID,
             bIsLast,
             pRspInfo->ErrorID,
             pRspInfo->ErrorMsg);
    } else if (pSpecificInstrument) {
        ILOG("Ctp UnSubMarketData success! RequestID:{},IsLast:{},InstrumentID:{}",
             nRequestID,
             bIsLast,
             pSpecificInstrument->InstrumentID);
    }
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_unsub_fun_) {
        std::invoke(on_unsub_fun_, pSpecificInstrument, pRspInfo);
    }
}

void CtpMdSpi::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField* pSpecificInstrument,
                                   CThostFtdcRspInfoField*            pRspInfo,
                                   int32                              nRequestID,
                                   bool                               bIsLast) {
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        ELOG("Ctp SubForQuote failed! RequestID:{},IsLast:{},ErrorID:{},ErrorMsg:{}",
             nRequestID,
             bIsLast,
             pRspInfo->ErrorID,
             pRspInfo->ErrorMsg);
    } else if (pSpecificInstrument) {
        ILOG("Ctp SubForQuote success! RequestID:{},IsLast:{},InstrumentID:{}",
             nRequestID,
             bIsLast,
             pSpecificInstrument->InstrumentID);
    }
}

void CtpMdSpi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField* pSpecificInstrument,
                                     CThostFtdcRspInfoField*            pRspInfo,
                                     int32                              nRequestID,
                                     bool                               bIsLast) {
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        ELOG("Ctp UnSubForQuote failed! RequestID:{},IsLast:{},ErrorID:{},ErrorMsg:{}",
             nRequestID,
             bIsLast,
             pRspInfo->ErrorID,
             pRspInfo->ErrorMsg);
    } else if (pSpecificInstrument) {
        ILOG("Ctp UnSubForQuote success! RequestID:{},IsLast:{},InstrumentID:{}",
             nRequestID,
             bIsLast,
             pSpecificInstrument->InstrumentID);
    }
}

void CtpMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData) {
    //ELOG("Ctp receive MarketData.");
    if (pDepthMarketData) {
        DLOG("Ctp receive MarketData.");
    }
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_data_fun_) {
        std::invoke(on_data_fun_, pDepthMarketData);
        //ELOG("Ctp receive");
    }
}

void CtpMdSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField* pForQuoteRsp) {
    if (pForQuoteRsp) {
        DLOG("Ctp receive QuoteRsp.");
    }
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_quote_fun_) {
        std::invoke(on_quote_fun_, pForQuoteRsp);
    }
}

void CtpMdSpi::clearCallback() {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_connected_fun_ = {};
    on_disconnected_fun_ = {};
    on_login_fun_ = {};
    on_error_fun_ = {};
    on_sub_fun_ = {};
    on_unsub_fun_ = {};
    on_data_fun_ = {};
    on_quote_fun_ = {};
}
