// Standard library
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// 3rd part library
#include <curl/curl.h>

#include "stockDataHandler.h"
#include "safequeue.h"

#include "_DECL_NAMESPACE_BEGIN.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::pair;
using std::string;
using std::stringstream;
using std::vector;

const char *const TWSE_001_URL = "https://www.twse.com.tw/indicesReport/MI_5MINS_HIST?response=json&date=";
const size_t SIZE_OF_INPUT_NUMBER = 128;

namespace
{
    inline size_t
    callback(
        void *buffer,
        size_t size,
        size_t nmemb,
        void *userp)
    {
        string &rResponse = *static_cast<string *>(userp);
        if (size * nmemb < 1)
        {
            return 0;
        }
        else
        {
            rResponse.append(
                static_cast<const char *>(buffer),
                size * nmemb);

            return size * nmemb;
        }
    }

    inline void httpGet(
        const char *url,
        string *pResponse)
    {
        CURL *curl = curl_easy_init();
        if (curl)
        {
            CURLcode res;
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, pResponse);
            res = curl_easy_perform(curl);
            if (CURLE_OK != res)
            {
                stringstream ss;
                ss << "curl error (" << res << "), "
                << curl_easy_strerror(res);

                throw std::runtime_error(ss.str());
            }
            curl_easy_cleanup(curl);
        }
    }
} // unnamed namespace

int main_i(int argc, char *argv[])
{
    try
    {
        int num;

        cout << "Please input the number of stock data, and it should be postive number:" << endl;
        cin >> num;
        if (cin.fail())
        {
            throw std::runtime_error("input numner format error!"); //Not an int.
        }
        cin.ignore(1024, '\n');

        if (num < 1)
        {
            return -1;
        }

        cout << "Please input " << num
             << " dates sequentially using format,"
             << " for example: <2002 2 1>" << endl;

        vector<Date_t> vecInputDate;
        map<YearMonth_t, vector<int>> monthMap;
        for (int i = 0; i < num; ++i)
        {
            char input[SIZE_OF_INPUT_NUMBER];
            cin.getline(input, sizeof(input));

            // input format check
            for (int j = 0; input[j] != '\0' && j < SIZE_OF_INPUT_NUMBER; ++j)
            {
                if ((input[j] != 32 && input[j] < '0') || '9' < input[j])
                {
                    throw std::runtime_error("date format error!");
                }
            }

            // format check complete!
            // continue to split string
            stringstream ss(input);
            vector<int> vecInput;
            int t;
            while (ss >> t)
            {
                vecInput.push_back(t);
                t = 0;
            }

            if (vecInput.size() != 3)
            {
                throw std::runtime_error("date format error!");
            }

            // 限制僅能輸入西元 1999~2021 年
            if (vecInput[0] < 1999 || 2021 < vecInput[0])
            {
                throw std::runtime_error("year format error!");
            }

            // 月份僅能輸入 1~12
            if (vecInput[1] < 1 || 12 < vecInput[1])
            {
                throw std::runtime_error("month format error!");
            }

            // 日僅能輸入 1~31
            if (vecInput[1] < 1 || 31 < vecInput[2])
            {
                throw std::runtime_error("day format error!");
            }

            Date_t t2(vecInput[0], vecInput[1], vecInput[2]);
            vecInputDate.push_back(t2);
            monthMap[std::make_pair(vecInput[0], vecInput[1])].push_back(vecInput[2]);
        }

        auto spSafeQueue = std::make_shared<SafeQueue<SafeQueueData>>();
        StockDataHandler handler(vecInputDate, monthMap, spSafeQueue);
        handler.init();

        for (const auto &it : monthMap)
        {
            char url[100];
            sprintf(url, "%s%d%02d01", TWSE_001_URL, it.first.first, it.first.second);
            auto spResponse = std::make_shared<string>();
            httpGet(url, spResponse.get()); // 發送http request取得股價資料
            SafeQueueData safeQueueData(SafeQueueData::DATA, it.first, spResponse);
            spSafeQueue->enqueue(safeQueueData);
        }

        auto spResponse = std::make_shared<string>();
        SafeQueueData safeQueueData;
        spSafeQueue->enqueue(safeQueueData);

        handler.fini();

        return 0;
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

    return -1;
}

#include "_DECL_NAMESPACE_END.h"

int main(int argc, char *argv[])
{
    return CodeTest::main_i(argc, argv);
}