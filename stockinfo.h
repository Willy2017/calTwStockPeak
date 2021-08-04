// -*- C++ -*-

//=============================================================================
/**
 * @file stockinfo.h
 *
 * @author Bofu Huang
 *
 * @date 2021-04-21
 */
//=============================================================================

#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <map>

#include "_DECL_NAMESPACE_BEGIN.h"

using std::cout;
using std::endl;
using std::string;

class StockInfo
{
public:

    enum Type
    {
        TSE, //台灣上市股票
        OTC  //台灣上櫃股票
    };

    StockInfo() {}
    StockInfo(
        struct tm *pTime,
        const float startPrice = 0,
        const float maxPrice = 0,
        const float minPrice = 0,
        const float endPrice = 0)
    :
        mupTime(pTime)
      , mStartPrice(startPrice)
      , mMaxPrice(maxPrice)
      , mMinPrice(minPrice)
      , mEndPrice(endPrice)
    {}

    ~StockInfo() {}

    void printStockInfo()
    {
        cout << (mupTime->tm_year + 1900) << '-'
             << (mupTime->tm_mon + 1) << '-'
             << mupTime->tm_mday << ": "
             << getStockPeakInterval() << endl;
    }

    float getStockPeakInterval() { return mMaxPrice - mMinPrice; }

private:


    std::unique_ptr<struct tm> mupTime; //股票時間
    float mStartPrice;                  //開盤價
    float mMaxPrice;                    //當日最高價
    float mMinPrice;                    //當日最低價
    float mEndPrice;                    //收盤價
};

typedef int StockType;
typedef string CodeName_t;  // 股票代號, e.g. 台積電股票代號 2330, 這裡我們暫時定義加權指數為001
typedef std::pair<StockType, CodeName_t> StockKey;

typedef std::shared_ptr<StockInfo> StockInfoPtr;
typedef std::vector<StockInfoPtr>  StockInfoList;
typedef std::map<StockKey, StockInfoList> StockInfoListMap;

#include "_DECL_NAMESPACE_END.h"