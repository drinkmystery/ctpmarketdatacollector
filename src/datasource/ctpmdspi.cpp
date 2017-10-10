﻿#include "ctpmdspi.h"

#include "utils/logger.h"

void CtpMdSpi::setOnFrontConnected(std::function<void()>&& fun) {
    on_connected_fun_ = fun;
}

void CtpMdSpi::setOnFrontDisConnected(std::function<void(int32)>&& fun) {
    on_disconnected_fun_ = fun;
}

void CtpMdSpi::setOnLoginFun(std::function<void(CThostFtdcRspUserLoginField*, CThostFtdcRspInfoField*)> fun) {
    on_login_fun_ = fun;
}

void CtpMdSpi::setOnErrorFun(std::function<void(CThostFtdcRspInfoField*)> fun) {
    on_error_fun_ = fun;
}

void CtpMdSpi::setOnSubFun(std::function<void(CThostFtdcSpecificInstrumentField*, CThostFtdcRspInfoField*)> fun) {
    on_sub_fun_ = fun;
}

void CtpMdSpi::setOnUnSubFun(std::function<void(CThostFtdcSpecificInstrumentField*, CThostFtdcRspInfoField*)> fun) {
    on_unsub_fun_ = fun;
}

void CtpMdSpi::setOnDataFun(std::function<void(CThostFtdcDepthMarketDataField*)> fun) {
    on_data_fun_ = fun;
}

void CtpMdSpi::setOnQuoteFun(std::function<void(CThostFtdcForQuoteRspField*)> fun) {
    on_quote_fun_ = fun;
}

void CtpMdSpi::OnFrontConnected() {
    ILOG("Ctp connected to front.");
    if (on_connected_fun_) {
        std::invoke(on_connected_fun_);
    }
}

void CtpMdSpi::OnFrontDisconnected(int32 nReason) {
    ILOG("Ctp disconnect from front! Reason:{}", nReason);
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
        ILOG("Ctp Login success! RequestID:{},IsLast:{}", nRequestID, bIsLast);
    }

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
        ILOG("Ctp SubMarketData success! RequestID:{},IsLast:{},InstrumentID:{}",
             nRequestID,
             bIsLast,
             pSpecificInstrument->InstrumentID);
    }
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
    if (on_unsub_fun_) {
        std::invoke(on_sub_fun_, pSpecificInstrument, pRspInfo);
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
    if (pDepthMarketData) {
        DLOG("Ctp receive MarketData.");
    }
    if (on_data_fun_) {
        std::invoke(on_data_fun_, pDepthMarketData);
    }
}

void CtpMdSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField* pForQuoteRsp) {
    if (pForQuoteRsp) {
        DLOG("Ctp receive QuoteRsp.");
    }
    if (on_quote_fun_) {
        std::invoke(on_quote_fun_, pForQuoteRsp);
    }
}
