#include "CtpTdSpi.h"

#include "utils/logger.h"

void CtpTdSpi::OnFrontConnected() {}

void CtpTdSpi::OnFrontDisconnected(int nReason) {}

void ctpTdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField*      pRspInfo,
                              int                          nRequestID,
                              bool                         bIsLast) {

    if (pRspInfo && pRspInfo->ErrorID != 0) {
        ELOG("Ctp Login failed! RequestID:{},IsLast:{},ErrorID:{},ErrorMsg:{}",
             nRequestID,
             bIsLast,
             pRspInfo->ErrorID,
             pRspInfo->ErrorMsg);
    } else if (pRspUserLogin && bIsLast) {
        ILOG("Ctp Login success! RequestID:{},IsLast:{},ErrorId:{}", nRequestID, bIsLast, pRspInfo->ErrorID);
        front_id_   = pRspUserLogin->FrontID;
        session_id_ = pRspUserLogin->SessionID;
        order_ref_  = atoi(pRspUserLogin->MaxOrderRef);
        trade_day_  = string(pRspUserLogin->TradingDay);
        order_ref_++;
    }

    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_login_fun_) {
        std::invoke(on_login_fun_, pRspUserLogin, pRspInfo);
    }
}

void CtpTdSpi::ReqQrySettlementInfoConfirm() {
    std::lock_guard<utils::spinlock>        guard(lock_);
    CThostFtdcQrySettlementInfoConfirmField req;
    memset(&req, 0, sizeof(req));
    strcpy_s(req.BrokerID, sizeof(TThostFtdcBrokerIDType), ctp_config_.brokerId);
    strcpy_s(req.InvestorID, sizeof(TThostFtdcInvestorIDType), ctp_config_.userId);
    int iResult = ctpTdApi_->ReqQrySettlementInfoConfirm(&req, ++request_id_);
    ILOG("ReqSettlementInforConrim,Result:{},requestId:{}.", iResult, request_id_);
}

void CtpTdSpi::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
                                             CThostFtdcRspInfoField*               pRspInfo,
                                             int                                   nRequestID,
                                             bool                                  bIsLast) {
    if (!IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm) {
        ILOG("OnRspSettlementInfoConfirm: Success!ConfirmDate:{},ConfirmTime:{},settlementID:{},nRequestID:{}.",
             pSettlementInfoConfirm->ConfirmDate,
             pSettlementInfoConfirm->ConfirmTime,
             pSettlementInfoConfirm->SettlementID,
             nRequestID);
    }
    if (bIsLast) {
        if (pSettlementInfoConfirm) {
            ReqQryInstrument_all();
        } else {
            ReqSettlementInfoConfirm();
        }
    }
}

void CtpTdSpi::ReqSettlementInfoConfirm() {
    std::lock_guard<std::mutex>          guard(mut_);
    CThostFtdcSettlementInfoConfirmField req;
    memset(&req, 0, sizeof(req));
    strcpy_s(req.BrokerID, sizeof(TThostFtdcBrokerIDType), ctp_config_.brokerId);
    strcpy_s(req.InvestorID, sizeof(TThostFtdcInvestorIDType), ctp_config_.userId);
    int iResult = ctpTdApi_->ReqSettlementInfoConfirm(&req, ++request_id_);
    ILOG("First ReqSettlementInforConrim,Result:{}.request_id:{}.", iResult, request_id_);
}

void CtpTdSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
                                          CThostFtdcRspInfoField*               pRspInfo,
                                          int                                   nRequestID,
                                          bool                                  bIsLast) {

    if (!IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm) {

        ILOG("OnRspSettlementInfoConfirm: Success!ConfirmDate:{},ConfirmTime:{},settlementID:{},nRequestID:{}.",
             pSettlementInfoConfirm->ConfirmDate,
             pSettlementInfoConfirm->ConfirmTime,
             pSettlementInfoConfirm->SettlementID,
             nRequestID);
    }
    if (bIsLast) {
        ReqQryInstrument_all();
    }
}

void CtpTdSpi::ReqQryInstrument_all() {
    std::lock_guard<std::mutex>  guard(mut_);
    CThostFtdcQryInstrumentField req;
    memset(&req, 0, sizeof(req));
    while (true) {
        int iResult = ctpTdApi_->ReqQryInstrument(&req, ++request_id_);
        ILOG("ReqQryInstrument, result:{},requestId:{}.", iResult, request_id_);
        if (IsFlowControl(iResult)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        } else {
            break;
        }
    }
}

void CtpTdSpi::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument,
                                  CThostFtdcRspInfoField*    pRspInfo,
                                  int                        nRequestID,
                                  bool                       bIsLast) {

    if (!IsErrorRspInfo(pRspInfo) && pInstrument) {
        if (m_first_inquiry_Instrument == true) {
            // save all instrument to Inst message map
            auto instField = make_shared<CThostFtdcInstrumentField>(*pInstrument);
            // maybe save all Insturment msg is better?but save to csv or save to database?
            m_InstMeassageMap->insert(
                pair<string, std::shared_ptr<CThostFtdcInstrumentField>>(instField->InstrumentID, instField));
        }
    }
    if (bIsLast) {
        m_first_inquiry_Instrument = false;
        ILOG("OnRspQryInstrument,bIsLast:{},nRequestID:{}.", bIsLast, nRequestID);
        ReqQryOrder();
    }
}
void CtpTdSpi::ReqSettlementInfoConfirm() {}

bool CtpTdSpi::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo) {}

void CtpTdSpi::clearCallBack() {}