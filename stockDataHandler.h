// -*- C++ -*-

//=============================================================================
/**
 * @file stockHandler.h
 *
 * @author Bofu Huang
 *
 * @date 2021-04-21
 */
//=============================================================================

#pragma once

#include <map>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "stockinfo.h"
#include "safequeue.h"

#include "_DECL_NAMESPACE_BEGIN.h"

using std::map;
using std::shared_ptr;
using std::string;
using std::thread;
using std::vector;

typedef std::tuple<int, int, int> Date_t;
typedef std::pair<int, int> YearMonth_t;

class SafeQueueData
{
public:
    enum Type
    {
        EOQ, //End Of Queue
        DATA
    };

    SafeQueueData(
        const int safeQueueDataType = EOQ
      , const YearMonth_t& yearMonth = std::make_pair(0,0)
      , shared_ptr<string> spResponseStr = std::make_shared<string>())
    :
        mSafeQueueDataType(safeQueueDataType)
      , mYearMonth(yearMonth)
      , mspResponseStr(spResponseStr)
    {}

    ~SafeQueueData() {}

    int getType() { return mSafeQueueDataType; }
    const YearMonth_t& getYearMonth() { return mYearMonth; }
    shared_ptr<const string> getResponseStr() { return mspResponseStr; }

private:
    int mSafeQueueDataType;
    YearMonth_t mYearMonth;
    shared_ptr<string> mspResponseStr;
};
typedef int SafeQueueDataType;

class StockDataHandler
{
public:
    StockDataHandler(
        const vector<Date_t> &
      , const map<YearMonth_t, vector<int>> &
      , shared_ptr<SafeQueue<SafeQueueData>>);

    ~StockDataHandler() {}

    void onParse();

    void init(void);

    void fini(void);

private:
    vector<Date_t> mVecInputDate;  //輸入的日期
    map<YearMonth_t, vector<int>> mMonthMap;       //數入的日期以月分集合起來的map，對應的value為該月分輸入的日
    shared_ptr<thread> mspWorker;
    shared_ptr<SafeQueue<SafeQueueData>> mspQueue;
    map<string, vector<float>> mResponseMap;       // http get所得到的答案的集合
    StockInfoListMap mStockInfoListMap;            //最終儲存的股票相關資訊
};

#include "_DECL_NAMESPACE_END.h"
