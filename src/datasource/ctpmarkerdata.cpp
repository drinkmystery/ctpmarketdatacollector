#include "ctpmarkerdata.h"

#include <future>

#include <boost/algorithm/string.hpp>

#include "utils/logger.h"

int32 CtpMarketData::init(const CtpConfig& ctp_config) {
    is_inited_ = false;

    // 1. Create Ctp Api Instance.
    {
        auto mdapi = CThostFtdcMdApi::CreateFtdcMdApi(ctp_config.flow_path.c_str());
        if (mdapi == nullptr) {
            ELOG("Ctp create api instance failed!");
            return -1;
        }
        ctpmdapi_ = {mdapi, [](CThostFtdcMdApi* mdapi) {
            // The Ctp's Document does not say how to release the api gracefully, maybe Release then Join, NEED TEST! 2017/10/10 drinkmystery
            if (mdapi != nullptr) {
                mdapi->Release();
            }
            // The actul result is cannot call Join after Release! 2017/10/11 drinkmystery
            //if (mdapi != nullptr) {
            //    mdapi->Join();
            //}
        }};
        ctpmdapi_->RegisterSpi(&ctpmdspi_);
        ILOG("Ctp create api instance success!");
    }
    // 2. Connect to Ctp Front.
    {
        // Prepare.
        ctpmdapi_->RegisterFront(const_cast<char*>(ctp_config.md_address.c_str()));
        std::promise<bool> connect_result;
        std::future<bool>  is_connected = connect_result.get_future();
        ctpmdspi_.setOnFrontConnected([&connect_result] { connect_result.set_value(true); });
        ctpmdspi_.setOnErrorFun([&connect_result](CThostFtdcRspInfoField* rsp) {
            if (rsp->ErrorID != 0) {
                connect_result.set_value(false);
            }
        });
        // Try connect.
        ctpmdapi_->Init();
        is_connected.wait();
        if (is_connected.get() == false) {
            ctpmdspi_.setOnFrontConnected({});
            ctpmdspi_.setOnErrorFun({});
            return -2;
        }
        ctpmdspi_.setOnFrontConnected({});
        ctpmdspi_.setOnErrorFun({});
    }
    // 3. Login to Ctp.
    {
        // Prepare
        std::promise<bool> login_result;
        std::future<bool>  is_logined = login_result.get_future();
        ctpmdspi_.setOnLoginFun([&login_result](CThostFtdcRspUserLoginField* login, CThostFtdcRspInfoField* info) {
            if (info->ErrorID == 0) {
                login_result.set_value(true);
            } else {
                login_result.set_value(false);
            }
        });
        ctpmdspi_.setOnErrorFun([&login_result](CThostFtdcRspInfoField* rsp) {
            if (rsp->ErrorID != 0) {
                login_result.set_value(false);
            }
        });
        CThostFtdcReqUserLoginField req;
        ctp_config.broker_id.copy(req.BrokerID, sizeof(req.BrokerID));
        req.BrokerID[sizeof(req.BrokerID) - 1] = '\n';
        ctp_config.user_id.copy(req.UserID, sizeof(req.UserID));
        req.UserID[sizeof(req.UserID) - 1] = '\n';
        ctp_config.password.copy(req.Password, sizeof(req.Password));
        req.Password[sizeof(req.Password) - 1] = '\n';
        // Try Login
        auto call_result = ctpmdapi_->ReqUserLogin(&req, ++request_id_);
        if (call_result != 0) {
            ELOG("Ctp reqquest Login error:{}", call_result);
            ctpmdspi_.setOnLoginFun({});
            ctpmdspi_.setOnErrorFun({});
            return -3;
        };
        is_logined.wait();
        if (is_logined.get() == false) {
            ctpmdspi_.setOnLoginFun({});
            ctpmdspi_.setOnErrorFun({});
            return -3;
        }
        ctpmdspi_.setOnLoginFun({});
        ctpmdspi_.setOnErrorFun({});
    }
    // 4. Set Spi callback.
    {
        ctpmdspi_.setOnDataFun([this](CThostFtdcDepthMarketDataField* data) { buffer_.push(*data); });
        ctpmdspi_.setOnSubFun([this](CThostFtdcSpecificInstrumentField* instrument, CThostFtdcRspInfoField* rsp) {
            if (instrument != nullptr && rsp != nullptr && rsp->ErrorID == 0) {
                inst_ids.insert(instrument->InstrumentID);
            }
        });
        ctpmdspi_.setOnUnSubFun([this](CThostFtdcSpecificInstrumentField* instrument, CThostFtdcRspInfoField* rsp) {
            if (instrument != nullptr && rsp != nullptr && rsp->ErrorID == 0) {
                inst_ids.erase(instrument->InstrumentID);
            }
        });
    }

    is_inited_ = true;
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
        return ctpmdapi_->SubscribeMarketData(&(char_instrument_ids[0]), static_cast<int32>(char_instrument_ids.size()));
    }
    return 0;
}

bool CtpMarketData::getData(CThostFtdcDepthMarketDataField& data) {
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
    if (ctpmdapi_) {
        ctpmdapi_.reset(nullptr);
    }
    is_inited_ = false;
    return 0;
}
