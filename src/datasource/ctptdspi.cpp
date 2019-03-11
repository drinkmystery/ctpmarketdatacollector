#include "ctptdspi.h"

#include "utils/logger.h"

void ctpTdSpi::OnFrontConnected() {


}

void ctpTdSpi::OnFrontDisconnected(int nReason) {

}

void ctpTdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField*      pRspInfo,
                              int                          nRequestID,
                              bool                         bIsLast) {
                                // std::lock_guard<std::mutex> guard(mut_);
                                if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin) {
                                    m_FRONT_ID        = pRspUserLogin->FrontID;
                                    m_SESSION_ID      = pRspUserLogin->SessionID;
                                    int iNextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
                                    m_tradeDay        = string(pRspUserLogin->TradingDay);
                                    m_actionDay       = "";  // cSystem::GetCurrentDayBuffer();

                                    iNextOrderRef++;
                                    sprintf(m_ORDER_REF, "%d", iNextOrderRef);

                                    ILOG("cTraderSpi::OnRspUserLogin,TradeDate:{},SessionID:{},FrontID:{},MaxOrderRef:{}.",
                                        pRspUserLogin->TradingDay,
                                        pRspUserLogin->SessionID,
                                        pRspUserLogin->FrontID,
                                        pRspUserLogin->MaxOrderRef);
                                }
                                if (bIsLast) {
                                    if (on_login_fun_) {
                                        std::invoke(on_login_fun_, pRspUserLogin, pRspInfo);
                                    }
                                }

                              }

void ctpTdSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
                                          CThostFtdcRspInfoField*               pRspInfo,
                                          int                                   nRequestID,
                                          bool                                  bIsLast) {}

// query instrument response
void ctpTdSpi::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument,
                                  CThostFtdcRspInfoField*    pRspInfo,
                                  int                        nRequestID,
                                  bool                       bIsLast) {}

void ctpTdSpi::ReqQrySettlementInfoConfirm() {}

void ctpTdSpi::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
                                             CThostFtdcRspInfoField*               pRspInfo,
                                             int                                   nRequestID,
                                             bool                                  bIsLast) {}

void ctpTdSpi::ReqSettlementInfoConfirm() {}

bool ctpTdSpi::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo) {}