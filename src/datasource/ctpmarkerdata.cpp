#include "ctpmarkerdata.h"

#include <future>
#include <chrono>
#include <iostream>

#include <boost/algorithm/string.hpp>

#include "utils/logger.h"
#include "utils/global.h"
#include "utils/scopeguard.h"

int32 CtpMarketData::init_md(const CtpConfig& ctp_config) {
    using namespace std::chrono_literals;
    is_inited_ = false;

    // 1. Create Ctp Api Instance.
    {
        auto mdapi = CThostFtdcMdApi::CreateFtdcMdApi(ctp_config.md_flow_path.c_str());
        // ELOG("flow_path:{}", ctp_config.flow_path.c_str());
        if (mdapi == nullptr) {
            ELOG("Ctp create api instance failed!");
            return -1;
        }
        ctpmdapi_ = {mdapi, [](CThostFtdcMdApi* mdapi) {
                         if (mdapi != nullptr) {
                             mdapi->Release();
                         }
                         ELOG("Release Mdapi");
                     }};
        ctpmdapi_->RegisterSpi(&ctpmdspi_);
        ILOG("Ctp create api instance success!");
    }
    // 2. Connect to Ctp Front.
    {
        // Prepare.
        ctpmdspi_.clearCallback();
        auto guard = utils::make_guard([this] { ctpmdspi_.clearCallback(); });

        ctpmdapi_->RegisterFront(const_cast<char*>(ctp_config.md_address.c_str()));

        std::promise<bool> connect_result;
        std::future<bool>  is_connected = connect_result.get_future();
        ctpmdspi_.setOnFrontConnected([&connect_result] { connect_result.set_value(true); });
        // Try connect.
        ctpmdapi_->Init();
        auto wait_result = is_connected.wait_for(5s);
        if (wait_result != std::future_status::ready || is_connected.get() != true) {
            return -2;
        }
    }
    // 3. Login to Ctp.
    {
        // Prepare
        ctpmdspi_.clearCallback();
        auto guard = utils::make_guard([this] { ctpmdspi_.clearCallback(); });

        std::promise<bool> login_result;
        std::future<bool>  is_logined = login_result.get_future();
        ctpmdspi_.setOnLoginFun([&login_result](CThostFtdcRspUserLoginField* login, CThostFtdcRspInfoField* info) {
            if (info->ErrorID == 0) {
                login_result.set_value(true);
            } else {
                login_result.set_value(false);
            }
        });

        CThostFtdcReqUserLoginField req;
        memset(&req, 0, sizeof(req));
        ctp_config.broker_id.copy(req.BrokerID, sizeof(req.BrokerID));
        req.BrokerID[sizeof(req.BrokerID) - 1] = '\n';
        ctp_config.user_id.copy(req.UserID, sizeof(req.UserID));
        req.UserID[sizeof(req.UserID) - 1] = '\n';
        ctp_config.password.copy(req.Password, sizeof(req.Password));
        req.Password[sizeof(req.Password) - 1] = '\n';
        // DLOG("BrokerId: {},UserId:{},PassWord:{},address:{}",
        //     req.BrokerID,
        //     req.UserID,
        //     req.Password,
        //     const_cast<char*>(ctp_config.md_address.c_str()));
        // Try Login
        auto call_result = ctpmdapi_->ReqUserLogin(&req, ++request_id_);
        if (call_result != 0) {
            ELOG("Ctp reqquest Login error:{}", call_result);
            return -3;
        };
        auto wait_result = is_logined.wait_for(5s);
        if (wait_result != std::future_status::ready || is_logined.get() != true) {
            return -3;
        }
    }
    // 4. Set Spi callback.
    {
        ctpmdspi_.clearCallback();
        ctpmdspi_.setOnDataFun([this](CThostFtdcDepthMarketDataField* data) {
            if (data != nullptr) {
                buffer_.push(MarketData(*data));
            }
        });
        ctpmdspi_.setOnSubFun([this](CThostFtdcSpecificInstrumentField* instrument, CThostFtdcRspInfoField* rsp) {
            if (instrument != nullptr && rsp != nullptr && rsp->ErrorID == 0) {
                inst_ids_.insert(instrument->InstrumentID);
            }
        });
        ctpmdspi_.setOnUnSubFun([this](CThostFtdcSpecificInstrumentField* instrument, CThostFtdcRspInfoField* rsp) {
            if (instrument != nullptr && rsp != nullptr && rsp->ErrorID == 0) {
                inst_ids_.erase(instrument->InstrumentID);
            }
        });
        global::need_reconnect.store(false, std::memory_order_release);
        ctpmdspi_.setOnFrontDisConnected([](int32) {
            WLOG("Ctp disconnet, try reconnect!");
            global::need_reconnect.store(true, std::memory_order_release);
        });
        this->ctp_config_ = ctp_config;
    }

    is_inited_ = true;
    return 0;
}

int32 CtpMarketData::init_td(const CtpConfig& ctp_config) {
    using namespace std::chrono_literals;
    // 1.create td api instance
    {
        auto tdapi = CThostFtdcTraderApi::CreateFtdcTraderApi(ctp_config.td_flow_path.c_str());
        if (tdapi == nullptr) {
            ELOG("CreateFtdcTraderApi instance failed");
            return -1;
        }
        // unique_ptr->ctp's document release just call Release api, release 2018/07/11 JinnTao
        ctptdapi_ = {tdapi, [](CThostFtdcTraderApi* tdapi) {
                         if (tdapi != nullptr) {
                             tdapi->Release();
                         }
                         ELOG("Release tradeApi.");
                     }};
        ctptdapi_->RegisterSpi(&ctptdspi_);

        ILOG("Td create instance success!");
    }

    // 2.connect to td Front
    {
        ctptdspi_.clearCallBack();

        ctptdapi_->RegisterFront(const_cast<char*>(ctp_config.td_address.c_str()));
        std::promise<bool> connect_result;
        std::future<bool>  is_connected = connect_result.get_future();
        ctptdspi_.setOnFrontConnected([&connect_result] { connect_result.set_value(true); });
        ctptdapi_->Init();
        auto wait_result = is_connected.wait_for(15s);
        if (wait_result != std::future_status::ready || is_connected.get() != true) {
            return -2;
        }
        ILOG("Td connect front success!");
        ctptdapi_->SubscribePrivateTopic(THOST_TERT_QUICK);  // Private QUICK recieve exchange send all msg after login
        ctptdapi_->SubscribePublicTopic(THOST_TERT_QUICK);   // Public QUICK recieve exchange send all msg after login
    }

    // 3.login to Td.
    {
        ctptdspi_.clearCallBack();
        std::promise<bool> login_result;
        std::future<bool>  is_logined = login_result.get_future();
        ctptdspi_.setOnLoginFun([&login_result](CThostFtdcRspUserLoginField* login, CThostFtdcRspInfoField* info) {
            if (info->ErrorID == 0) {
                login_result.set_value(true);
            } else {
                login_result.set_value(false);
            }
        });
        CThostFtdcReqUserLoginField req;

        memset(&req, 0, sizeof(req));
        strcpy_s(req.BrokerID, sizeof(TThostFtdcBrokerIDType), ctp_config.broker_id.c_str());
        strcpy_s(req.UserID, sizeof(TThostFtdcInvestorIDType), ctp_config.user_id.c_str());
        strcpy_s(req.Password, sizeof TThostFtdcPasswordType, ctp_config.password.c_str());

        // Try login
        auto req_login_result = ctptdapi_->ReqUserLogin(&req, ++request_id_);
        if (req_login_result != 0) {
            ELOG("Td request login failed!");
            return -3;
        }
        auto wait_result = is_logined.wait_for(5s);
        if (wait_result != std::future_status::ready || is_logined.get() != true) {
            ELOG("Td request login TimeOut!");
            return -4;
        }
        ILOG("Td login success");
    }
    // 3.Settlment info
    {
        std::promise<bool> qry_settle_result;
        std::future<bool>  is_finish_qry = qry_settle_result.get_future();
        bool               is_confirm;
        ctptdspi_.setOnQrySettlement([&is_confirm, &qry_settle_result](bool confirm) {
            ILOG("settlement result {} ", confirm);
            is_confirm = confirm;
            qry_settle_result.set_value(true);

        });

        CThostFtdcQrySettlementInfoConfirmField req;
        memset(&req, 0, sizeof(req));
        strcpy_s(req.BrokerID, sizeof(TThostFtdcBrokerIDType), ctp_config_.broker_id.c_str());
        strcpy_s(req.InvestorID, sizeof(TThostFtdcInvestorIDType), ctp_config_.user_id.c_str());

        int iResult = ctptdapi_->ReqQrySettlementInfoConfirm(&req, ++request_id_);
        ILOG("First ReqQrySettlementInfoConfirm,Result:{}.request_id:{}.", iResult, request_id_);

        auto wait_result = is_finish_qry.wait_for(5s);
        if (wait_result != std::future_status::ready || is_finish_qry.get() != true) {
            ILOG("ReqQrySettlementInfoConfirm time out.");
            return -5;
        }

        if (!is_confirm) {
            std::promise<bool> settlement_confirm;
            std::future<bool>  is_settlement_confirm = settlement_confirm.get_future();
            ctptdspi_.setOnSettlementConfirm([&settlement_confirm]() { settlement_confirm.set_value(true); });

            CThostFtdcSettlementInfoConfirmField req;
            memset(&req, 0, sizeof(req));
            strcpy_s(req.BrokerID, sizeof(TThostFtdcBrokerIDType), ctp_config_.broker_id.c_str());
            strcpy_s(req.InvestorID, sizeof(TThostFtdcInvestorIDType), ctp_config_.user_id.c_str());
            int iResult = ctptdapi_->ReqSettlementInfoConfirm(&req, ++request_id_);
            ILOG("First ReqSettlementInforConrim,Result:{}.request_id:{}.", iResult, request_id_);
            wait_result = is_settlement_confirm.wait_for(20s);
            if (wait_result != std::future_status::ready || is_settlement_confirm.get() != true) {
                ILOG("ReqQrySettlementInfoConfirm time out.");
                return -6;
            }
            ILOG("Settlement Confim");
        }
    }

    // 4.set callback
    {
        ctptdspi_.clearCallBack();
        global::need_reconnect.store(false,
                                     std::memory_order_release);  // current write/read cannot set this store back;
        ctptdspi_.setOnFrontDisConnected([](int32 reason) {
            ELOG("Td disconnect,try reconnect! reason:{}", reason);
            global::need_reconnect.store(true, std::memory_order_release);
        });
        this->ctp_config_ = ctp_config;
    }
    return 0;
}

int32 CtpMarketData::subscribeMarketData(const string& instrument_ids) {
    if (!is_inited_) {
        ELOG("CtpMarketData is not inited");
        return -1;
    }
    std::vector<string> splited;
    boost::split(splited, instrument_ids, boost::is_any_of(","), boost::token_compress_on);
    if (!splited.empty()) {
        std::vector<char*> char_instrument_ids;
        char_instrument_ids.reserve(splited.size());
        for (auto& instrument_id : splited) {
            char_instrument_ids.emplace_back(const_cast<char*>(instrument_id.c_str()));
        }
        return ctpmdapi_->SubscribeMarketData(&(char_instrument_ids[0]),
                                              static_cast<int32>(char_instrument_ids.size()));
    }
    return 0;
}

int32 CtpMarketData::subscribeMarketData(const std::vector<string>& instrument_ids) {
    if (!is_inited_) {
        ELOG("CtpMarketData is not inited");
        return -1;
    }
    std::vector<char*> char_instrument_ids;
    for (const auto& instrument_id : instrument_ids) {
        char_instrument_ids.emplace_back(const_cast<char*>(instrument_id.c_str()));
    }
    auto result =
        ctpmdapi_->SubscribeMarketData(&(char_instrument_ids[0]), static_cast<int32>(char_instrument_ids.size()));
    // auto result =
    //    ctpmdapi_->SubscribeMarketData(nullptr,0);

    ILOG("subScribe MarketData  result:{},size:{}", result, static_cast<int32>(char_instrument_ids.size()));
    return result;
}
int32 CtpMarketData::subscribeMarketData() {
    using namespace std::chrono_literals;
    if (!is_inited_) {
        ELOG("CtpMarketData is not inited");
        return -1;
    }
    std::vector<std::string> instrument_ids;
    std::promise<bool>       inst_result;
    std::future<bool>        is_obtain_inst = inst_result.get_future();

    ctptdspi_.setOnInstrumentIds([&inst_result, &instrument_ids](std::vector<std::string> inst_list) {
        inst_result.set_value(true);
        // copy assign
        instrument_ids = inst_list;
    });

    // ctptdspi_.ReqQrySettlementInfoConfirm();

    CThostFtdcQryInstrumentField req;
    memset(&req, 0, sizeof(req));
    // prevent flow control
    while (true) {
        int iResult = ctptdapi_->ReqQryInstrument(&req, ++request_id_);
        ILOG("ReqQryInstrument, result:{},requestId:{}.", iResult, request_id_);
        if (IsFlowControl(iResult)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        } else {
            break;
        }
    }
    auto wait_result = is_obtain_inst.wait_for(60s);
    if (wait_result != std::future_status::ready || is_obtain_inst.get() != true) {
        return -2;
    }
    ILOG("Obtain Inst list success");
    using namespace std;
    for (auto i = instrument_ids.begin(); i != instrument_ids.end(); i++) {
        std::cout << *i << " ";
    }
    this->subscribeMarketData(instrument_ids);
    return 0;
}
bool CtpMarketData::getData(MarketData& data) {
    if (!is_inited_) {
        ELOG("CtpMarketData is not inited");
        return false;
    }
    return buffer_.pop(data);
}

bool CtpMarketData::empty() {
    return buffer_.empty();
}

int32 CtpMarketData::stop() {

    // stop ctp md api
    if (ctpmdapi_) {
        ctpmdapi_.reset(nullptr);
    }
    inst_ids_.clear();
    is_inited_ = false;

    // stop ctp td api
    if (ctptdapi_) {
        ctptdapi_.reset(nullptr);
    }

    inst_ids_.clear();
    is_inited_ = false;

    return 0;
}

int32 CtpMarketData::reConnect(const CtpConfig& ctp_config) {
    auto result = stop();

    if (result != 0) {
        ELOG("Ctp reconnect failed while stoping pre instance! Result:{}", result);
        return -1;
    }

    result = init_td(ctp_config);
    if (result != 0) {
        ELOG("Ctp reconnect md init failed! Result:{}", result);
        return -2;
    }

    result = init_md(ctp_config);
    if (result != 0) {
        ELOG("Ctp reconnect md init failed! Result:{}", result);
        return -2;
    }

    return 0;
}

// according ctp doc
bool CtpMarketData::IsFlowControl(int iResult) {
    //      0，代表成功。

    //    - 1，表示网络连接失败；

    //    - 2，表示未处理请求超过许可数；

    //    - 3，表示每秒发送请求数超过许可数。

    return ((iResult == -2) || (iResult == -3));
}