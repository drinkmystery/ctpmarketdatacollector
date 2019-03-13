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