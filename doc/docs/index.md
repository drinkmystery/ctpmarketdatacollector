# 简单说明

## Build

### 前提
* 需要Boost, 解压boost.7z 解压路径记作path/to/boost
* 需要mongocxx driver 解压mongodriver.rar 解压路径记作path/to/mongodriver
* 需要cmake

### 具体编译步骤
1. 创建build文件夹
2. 
```
cd build && cmake -G"Visual Studio 14 2015 Win64" -DCMAKE_TOOLCHAIN_FILE="path/to/boost/......./scripts/buildsystems/vcpkg.cmake" -DCMAKE_PREFIX_PATH="path/to/mongodriver" -DBoost_NO_BOOST_CMAKE="ON" ..
```
3. 打开build下sln 文件 进入VS2015编译

## 运行
1. setting文件
2. dll文件
    1. boost_filesystem-vc141-mt-1_65_1.dll
    2. boost_program_options-vc141-mt-1_65_1.dll
    3. boost_system-vc141-mt-1_65_1.dll
    4. thostmduserapi.dll
3. mongodb初始化
    1. 先在mongdb里建立一个db 然后建立一个collection 测试一个合约 collection名称为合约对应的id
4. .\ctp-marketdata-collector.exe --conf setting.ini

## 修改

代码中有TODO的是需要修改的

1. src/utils/structures.h:23: 修改一条记录的变量 比如最高价之类的 按需要添加

```cpp
struct MarketData {
    // TODO
    string instrument_id;
    string date;
    string value;
    string value1;

    std::chrono::time_point<std::chrono::system_clock> last_tick_time;
    std::chrono::time_point<std::chrono::system_clock> last_update_time;
};
```

2. src/collector/ctpmarketdatacollector.cpp:206: 原始行情CThostFtdcDepthMarketDataField转换为MarketData

```cpp
// TODO add value and ticktime
while (!ctp_md_data_.empty()) {
    CThostFtdcDepthMarketDataField origin;
    if (ctp_md_data_.getData(origin)) {
        MarketData data = {origin.InstrumentID,
                            origin.UpdateTime,
                            std::to_string(origin.LastPrice),
                            std::to_string(origin.OpenPrice),
                            std::chrono::system_clock::now()};

        auto it = data_records_.find(data.instrument_id);
        if (it != data_records_.end()) {
            it->second = data;
        } else {
            data_records_.insert({data.instrument_id, data});
        }
    }
}
```

3. datastore/mongostore.cpp:57: MarketData记录进mongodb中

```cpp
for (size_t index = 0, read = buffer_.pop(&(datas[0]), count); index < read; ++index) {
    // TODO add more filed
    auto collection = db_.collection(datas[index].instrument_id);
    // clang-format off
    auto doc_value = bsoncxx::builder::stream::document{}
        << "id" << datas[index].instrument_id
        << "date" << datas[index].date
        << "value" << datas[index].value
        << bsoncxx::builder::stream::finalize;
    // clang-format on
    collection.insert_one(doc_value.view());
}
```
