# encoding: UTF-8

"""
导入MC导出的CSV历史数据到MongoDB中
"""

from datetime import datetime, timedelta
from time import time
from multiprocessing.pool import ThreadPool
import os
import pymongo
import re
mongoSetting = {
    "dbName":"futureData",
    "host":"localhost",
    "port":27017,
    }
class barData(object):
        #----------------------------------------------------------------------
    def __init__(self):
        
        self.id = ''     
        self.actionDate = ''
        self.actionTime = ''
        self.exchange = ''

        self.high  = 0
        self.close = 0
        self.open = 0
        self.low = 0
        self.volume = 0
        self.position = 0
        self.mdTradeDay = ''
        self.mdUpdateTime = ''
        self.recordTime = None
        

def loadCsvToMongoDb(filename,symbol,collection,mongoSetting):
    
    """将Multicharts导出的csv格式的历史数据插入到Mongo数据库中"""
    import csv
    import pymongo
    start = time()
    print u'开始读取CSV文件%s中的数据插入到%s的%s中' %(filename, mongoSetting["dbName"], collection)
    
    # 锁定集合，并创建索引
    client = pymongo.MongoClient(mongoSetting['host'], mongoSetting['port']) 
    dbName = mongoSetting["dbName"]
    collection = client[dbName][collection]
    collection.ensure_index([('recordTime', pymongo.ASCENDING)], unique=True)   
    
    # 读取数据和插入到数据库
    reader = csv.DictReader(file(filename, 'r'))
    for d in reader:
        bar = barData()
        bar.id = symbol
        bar.actionDate = datetime.strptime(d['Date'], '%Y/%m/%d').strftime('%Y%m%d')
        bar.actionTime = d['Time']
        bar.open = float(d['Open'])
        bar.high = float(d['High'])
        bar.low = float(d['Low'])
        bar.close = float(d['Close'])
        bar.volume = float(d['Volume'])
        bar.position = d['Position']

        bar.mdTradeDay = datetime.strptime(d['Date'], '%Y/%m/%d').strftime('%Y%m%d')
        bar.mdUpdateTime = d['Time']

        bar.recordTime = datetime.strptime(bar.actionDate + ' ' + bar.actionTime, '%Y%m%d %H:%M:%S')
       
        flt = {'recordTime': bar.recordTime}
        collection.update_one(flt, {'$set':bar.__dict__}, upsert=True)  
        print bar.actionDate, bar.actionTime
    
    print u'插入完毕，耗时：%s' % (time()-start)
if __name__ == '__main__':
   # loadMcCsv('IF0000_1min.csv', MINUTE_DB_NAME, 'IF0000')
    #loadMcCsv("rb0000_1min.csv", MINUTE_DB_NAME, 'rb0000')
    fileList = os.listdir(os.getcwd())
    filecollection = {}
    pattern = re.compile(r'$_1min.csv');

    #loadCsvToMongoDb("rb0000_1min.csv",'rb0000','rb0000',mongoSetting)
    for filename in fileList:
        `if pattern.match(filename):
            print filename

    print 'hello world',fileList
   