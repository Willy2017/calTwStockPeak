#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "safequeue.h"
#include "stockDataHandler.h"

#include "_DECL_NAMESPACE_BEGIN.h"

using std::cout;
using std::endl;
using std::map;
using std::pair;
using std::shared_ptr;
using std::string;
using std::stringstream;
using std::thread;
using std::vector;

const char SEP_SYMB = ',';

namespace
{
    inline struct tm *
    maketmp(int year, int month, int day) // e.g., year-month-day = 2010-01-01
    {
        //此處僅針對某一日期，時分秒不計
        struct tm *pRes = new struct tm;
        pRes->tm_year = year - 1900;
        pRes->tm_mon = month - 1;
        pRes->tm_mday = day;
        pRes->tm_hour = 0;
        pRes->tm_min = 0;
        pRes->tm_sec = 0;
        pRes->tm_isdst = 0;
        return pRes;
    }

    inline float getFloat(string a)
    {
        for (int i = 0; i < a.size(); ++i)
        {
            if (a[i] == SEP_SYMB)
            {
                a.erase(i, 1);
            }
        }

        return atof(a.c_str());
    }
} // unnamed namespace

StockDataHandler::StockDataHandler(
    const vector<Date_t> &vecInputDate
  , const map<YearMonth_t, vector<int>> &monthMap
  , shared_ptr<SafeQueue<SafeQueueData>> spQueue)
:
    mVecInputDate(vecInputDate)
  , mMonthMap(monthMap)
  , mspQueue(spQueue)
{
}

void StockDataHandler::onParse()
{
    try
    {
        StockInfoList stockInfoList;

        cout << endl
             << "output:" << endl;

        SafeQueueDataType flag;
        do
        {
            SafeQueueData data = mspQueue->dequeue();
            flag = data.getType();
            if (SafeQueueData::DATA == flag)
            {
                shared_ptr<const string> spResponse = data.getResponseStr();
                rapidjson::Document d;
                d.Parse(spResponse->c_str());
                const YearMonth_t &monthKey = data.getYearMonth();
                char date[10];
                sprintf(date, "%d%02d01", monthKey.first, monthKey.second);
                if (d["stat"].GetString() == string("OK") && d["date"].GetString() == string(date))
                {
                    for (const auto &v : d["data"].GetArray())
                    {
                        vector<float> vecPrice;
                        vecPrice.push_back(getFloat(v[1].GetString())); //開盤指數
                        vecPrice.push_back(getFloat(v[2].GetString())); //最高指數
                        vecPrice.push_back(getFloat(v[3].GetString())); //最低指數
                        vecPrice.push_back(getFloat(v[4].GetString())); //收盤指數
                        mResponseMap[v[0].GetString()] = vecPrice;
                    }
#if 0
                    // 此處的程式碼暫時不使用
                    // 也可選擇一邊處理一邊印出答案，但順序有可能會與input不太相同
                    // 因為map key有經過排序，而且證交所request一次是以一個月為主
                    // 為了避免同月份發出太多request，固集中同個月份的一起發一次
                    // 這邊選擇全部處理完後再照input順序輸出
                    for (const auto &it : mMonthMap[monthKey])
                    {
                        char temp[12];
                        sprintf(temp, "%3d/%02d/%02d", monthKey.first - 1911, monthKey.second, it);

                        if (mResponseMap.find(string(temp)) != mResponseMap.end())
                        {
                            const vector<float> &vecPrice = mResponseMap[temp];
                            StockInfoPtr spStockInfo;
                            spStockInfo.reset(
                                new StockInfo(
                                    maketmp(monthKey.first, monthKey.second, it),
                                    vecPrice[0],
                                    vecPrice[1],
                                    vecPrice[2],
                                    vecPrice[3]));
                            stockInfoList.push_back(spStockInfo); //也可以不存資料
#ifndef WITHDATE
                            cout << spStockInfo->getStockPeakInterval() << endl;
#else
                            spStockInfo->printStockInfo(); // 可使用此行印出相關日期
#endif
                        }
                    }
#endif
                }
            }
        } while (SafeQueueData::EOQ != flag);

        //選擇在此處處理是為了符合input的順序輸出
        for (const auto &it : mVecInputDate)
        {
            char temp[12];
            sprintf(temp, "%3d/%02d/%02d", std::get<0>(it) - 1911, std::get<1>(it), std::get<2>(it));

            if (mResponseMap.find(string(temp)) != mResponseMap.end())
            {
                const vector<float> &vecPrice = mResponseMap[temp];
                StockInfoPtr spStockInfo;
                spStockInfo.reset(
                    new StockInfo(
                        maketmp(std::get<0>(it), std::get<1>(it), std::get<2>(it)),
                        vecPrice[0],
                        vecPrice[1],
                        vecPrice[2],
                        vecPrice[3]));
                stockInfoList.push_back(spStockInfo);
            }
        }

        //暫定加權指數股票代號為001 此資訊可存可不存
        StockKey sKey(StockInfo::TSE, "001");
        mStockInfoListMap[sKey] = stockInfoList;

        for (const auto &it : mStockInfoListMap[sKey])
        {
#ifndef WITHDATE
            cout << it->getStockPeakInterval() << endl;
#else
            it->printStockInfo(); // 可使用此行印出相關日期資訊
#endif
        }
    }
    catch (std::runtime_error &e)
    {
        stringstream ss;
        ss.str("");
        ss << __func__ << "(): Runtime Error!"
           << " " << e.what();
        cout << ss.str() << endl;
    }
    catch (std::exception &e)
    {
        stringstream ss;
        ss.str("");
        ss << __func__ << "(): STD Exception!"
           << " " << e.what();
        cout << ss.str() << endl;
    }
    catch (...)
    {
        stringstream ss;
        ss.str("");
        ss << __func__ << "(): Unknown Exception!";
        cout << ss.str() << endl;
    }
}

void StockDataHandler::init(void)
{
    mspWorker.reset(new thread(&StockDataHandler::onParse, this));
}

void StockDataHandler::fini(void)
{
    mspWorker->join();
}

#include "_DECL_NAMESPACE_END.h"