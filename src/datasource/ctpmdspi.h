#ifndef _CTPMDSPI_H_
#define _CTPMDSPI_H_

#include <functional>

#include <sfitctp/ThostFtdcMdApi.h>

#include "utils/common.h"
#include "utils/spinlock.h"

class CtpMdSpi : public CThostFtdcMdSpi {
public:
    CtpMdSpi() = default;
    ~CtpMdSpi() = default;

    void clearCallback();
    void setOnFrontConnected(std::function<void()>&& fun);
    void setOnFrontDisConnected(std::function<void(int32)>&& fun);
    void setOnLoginFun(std::function<void(CThostFtdcRspUserLoginField*, CThostFtdcRspInfoField*)> fun);
    void setOnErrorFun(std::function<void(CThostFtdcRspInfoField*)> fun);
    void setOnSubFun(std::function<void(CThostFtdcSpecificInstrumentField*, CThostFtdcRspInfoField*)> fun);
    void setOnUnSubFun(std::function<void(CThostFtdcSpecificInstrumentField*, CThostFtdcRspInfoField*)> fun);
    void setOnDataFun(std::function<void(CThostFtdcDepthMarketDataField*)> fun);
    void setOnQuoteFun(std::function<void(CThostFtdcForQuoteRspField*)> fun);

    ///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    virtual void OnFrontConnected();

    ///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
    ///@param nReason 错误原因
    ///        0x1001 网络读失败
    ///        0x1002 网络写失败
    ///        0x2001 接收心跳超时
    ///        0x2002 发送心跳失败
    ///        0x2003 收到错误报文
    virtual void OnFrontDisconnected(int32 nReason);

    ///心跳超时警告。当长时间未收到报文时，该方法被调用。
    ///@param nTimeLapse 距离上次接收报文的时间
    virtual void OnHeartBeatWarning(int32 nTimeLapse);

    ///登录请求响应
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                                CThostFtdcRspInfoField*      pRspInfo,
                                int32                        nRequestID,
                                bool                         bIsLast);

    ///登出请求响应
    virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
                                 CThostFtdcRspInfoField*    pRspInfo,
                                 int32                      nRequestID,
                                 bool                       bIsLast);

    ///错误应答
    virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo, int32 nRequestID, bool bIsLast);

    ///订阅行情应答
    virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument,
                                    CThostFtdcRspInfoField*            pRspInfo,
                                    int32                              nRequestID,
                                    bool                               bIsLast);

    ///取消订阅行情应答
    virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument,
                                      CThostFtdcRspInfoField*            pRspInfo,
                                      int32                              nRequestID,
                                      bool                               bIsLast);

    ///订阅询价应答
    virtual void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField* pSpecificInstrument,
                                     CThostFtdcRspInfoField*            pRspInfo,
                                     int32                              nRequestID,
                                     bool                               bIsLast);

    ///取消订阅询价应答
    virtual void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField* pSpecificInstrument,
                                       CThostFtdcRspInfoField*            pRspInfo,
                                       int32                              nRequestID,
                                       bool                               bIsLast);

    ///深度行情通知
    virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData);

    ///询价通知
    virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField* pForQuoteRsp);

private:
    utils::spinlock lock_;
    std::function<void()>      on_connected_fun_;
    std::function<void(int32)> on_disconnected_fun_;
    std::function<void(CThostFtdcRspUserLoginField*, CThostFtdcRspInfoField*)> on_login_fun_;
    std::function<void(CThostFtdcRspInfoField*)> on_error_fun_;
    std::function<void(CThostFtdcSpecificInstrumentField*, CThostFtdcRspInfoField*)> on_sub_fun_;
    std::function<void(CThostFtdcSpecificInstrumentField*, CThostFtdcRspInfoField*)> on_unsub_fun_;
    std::function<void(CThostFtdcDepthMarketDataField*)> on_data_fun_;
    std::function<void(CThostFtdcForQuoteRspField*)>     on_quote_fun_;

    DISALLOW_COPY_AND_ASSIGN(CtpMdSpi);
};

#endif  // _CTPMDSPI_H_
