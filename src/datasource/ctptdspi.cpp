#include "CtpTdSpi.h"

#include "utils/logger.h"

#define TEST

void CtpTdSpi::OnFrontConnected() {
    ILOG("Td Ctp connected to front.");
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_connected_fun_) {
        std::invoke(on_connected_fun_);
    }
}

void CtpTdSpi::OnFrontDisconnected(int nReason) {
    ILOG("Td Ctp disconnect from front! Reason:{}", nReason);
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_disconnected_fun_) {
        std::invoke(on_disconnected_fun_, nReason);
    }
}
// after login, request query settlement and query insturment meg
void CtpTdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
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
        front_id_     = pRspUserLogin->FrontID;
        broker_id_    = pRspUserLogin->BrokerID;
        session_id_   = pRspUserLogin->SessionID;
        inverstor_id_ = pRspUserLogin->UserID;
        order_ref_    = atoi(pRspUserLogin->MaxOrderRef);
        trade_day_    = string(pRspUserLogin->TradingDay);
        order_ref_++;

        // this->ReqQrySettlementInfoConfirm();
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
    strcpy_s(req.BrokerID, sizeof(TThostFtdcBrokerIDType), broker_id_.c_str());
    strcpy_s(req.InvestorID, sizeof(TThostFtdcInvestorIDType), inverstor_id_.c_str());
    int iResult = ctpTdApi_->ReqQrySettlementInfoConfirm(&req, ++request_id_);
    ILOG("First ReqQrySettlementInfoConfirm,Result:{}.request_id:{}.", iResult, request_id_);
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
    if (bIsLast && on_qry_settlement_) {
        if (pSettlementInfoConfirm) {
            // 已经确认结算信息
            std::invoke(on_qry_settlement_, true);
        } else {
            std::invoke(on_qry_settlement_, false);
        }
    }
}
void CtpTdSpi::ReqSettlementInfoConfirm() {
    std::lock_guard<utils::spinlock>     guard(lock_);
    CThostFtdcSettlementInfoConfirmField req;
    memset(&req, 0, sizeof(req));
    strcpy_s(req.BrokerID, sizeof(TThostFtdcBrokerIDType), broker_id_.c_str());
    strcpy_s(req.InvestorID, sizeof(TThostFtdcInvestorIDType), inverstor_id_.c_str());
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
    if (bIsLast && on_settle_confirm_) {
        std::invoke(on_settle_confirm_);
    }
}




void CtpTdSpi::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument,
                                  CThostFtdcRspInfoField*    pRspInfo,
                                  int                        nRequestID,
                                  bool                       bIsLast) {

    if (!IsErrorRspInfo(pRspInfo) && pInstrument) {

        //// save all instrument to Inst message map
        auto instField = std::make_shared<CThostFtdcInstrumentField>(*pInstrument);
        //// maybe save all Insturment msg is better?but save to csv or save to database?
        // m_InstMeassageMap->insert(
        //    pair<string, std::shared_ptr<CThostFtdcInstrumentField>>(instField->InstrumentID, instField));
        instrument_ids_.emplace_back(instField->InstrumentID);
        //ILOG("InstrumentID:{}.", instField->InstrumentID);
        // std::cout >> instField->InstrumentID >> std::endl;
    }
    if (bIsLast && on_instrument_ids_) {
        ILOG("OnRspQryInstrument,bIsLast:{},nRequestID:{},inst:{}.", bIsLast, nRequestID, instrument_ids_.size());
        std::invoke(on_instrument_ids_, instrument_ids_);
    }
}

bool CtpTdSpi::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo) {
    bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
    if (bResult) {
        ELOG("ErrorID:{},ErrorMsg:{}.", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
    return bResult;
}

void CtpTdSpi::OnRspError(CThostFtdcRspInfoField* pRspInfo, int32 nRequestID, bool bIsLast) {
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

void CtpTdSpi::clearCallBack() {

    std::lock_guard<utils::spinlock> guard(lock_);
    on_connected_fun_    = {};
    on_disconnected_fun_ = {};
    on_login_fun_        = {};
    on_error_fun_        = {};
}

void CtpTdSpi::setOnFrontConnected(std::function<void()>&& fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_connected_fun_ = fun;
}

void CtpTdSpi::setOnFrontDisConnected(std::function<void(int32)>&& fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_disconnected_fun_ = fun;
}

void CtpTdSpi::setOnLoginFun(std::function<void(CThostFtdcRspUserLoginField*, CThostFtdcRspInfoField*)> fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_login_fun_ = fun;
}

void CtpTdSpi::setOnErrorFun(std::function<void(CThostFtdcRspInfoField*)> fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_error_fun_ = fun;
}

void CtpTdSpi::setOnInstrumentIds(std::function<void(std::vector<std::string>)> fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_instrument_ids_ = fun;
}

void CtpTdSpi::setOnQrySettlement(std::function<void(bool)> fun) 
{
    std::lock_guard<utils::spinlock> guard(lock_);
    on_qry_settlement_ = fun;
}
void CtpTdSpi::setOnSettlementConfirm(std::function<void()> fun){
    std::lock_guard<utils::spinlock> guard(lock_);
    on_settle_confirm_ = fun;
}