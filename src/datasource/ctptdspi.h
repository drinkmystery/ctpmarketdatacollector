#ifndef _CTPMDSPI_H_
#define _CTPMDSPI_H_

#include <functional>

#include "sfitctp/ThostFtdcTraderApi.h"

#include "utils/common.h"
#include "utils/spinlock.h"

class ctpTdSpi : public CThostFtdcTraderSpi {
public:
    ctpTdSpi() = default;

    ~ctpTdSpi() = default;

    // After making a succeed connection with the CTP server, the client should send the login request to the CTP
    // server.
    virtual void OnFrontConnected();

    // When the connection between client and the CTP server disconnected, the following function will be called
    virtual void OnFrontDisconnected(int nReason);

    // After receiving the login request from the client, the CTP server will send the following response to notify the
    // client whether the login success or not
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                                CThostFtdcRspInfoField*      pRspInfo,
                                int                          nRequestID,
                                bool                         bIsLast);

    // investor settlement information confirmation response
    virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
                                            CThostFtdcRspInfoField*               pRspInfo,
                                            int                                   nRequestID,
                                            bool                                  bIsLast);

    // query instrument response
    virtual void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument,
                                    CThostFtdcRspInfoField*    pRspInfo,
                                    int                        nRequestID,
                                    bool                       bIsLast);

    
    void ReqQrySettlementInfoConfirm();

    virtual void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
                                               CThostFtdcRspInfoField*               pRspInfo,
                                               int                                   nRequestID,
                                               bool                                  bIsLast);

    void ReqSettlementInfoConfirm();

    bool IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo);



private:
    utils::spinlock lock_;
    using CtpTdApiPtr = std::unique_ptr<CThostFtdcTraderApi, std::function<void(CThostFtdcTraderApi*)>>;
    CtpTdApiPtr                                                                ctpTdApi_;
    int32                                                                      request_id_ = 0;
    std::function<void()>                                                      on_connected_fun_;
    std::function<void(CThostFtdcRspUserLoginField*, CThostFtdcRspInfoField*)> on_login_fun_;
    std::function<void(int32)>                                                 on_disconnected_fun_;
    std::function<void()>                                                      on_started_fun_;
    int32 front_id_;
    int32 session_id_;
    string trade_day_;
    int32 order_ref_;
};

#endif  // _CTPMDSPI_H_
