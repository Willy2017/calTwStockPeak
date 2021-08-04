# calTwStockPeak
計算台灣股市指定日期加權指數的最低與最高差值

### 使用到以下3rd library
- libcurl
- rapidjson

如果沒有安裝，ubuntu系列系統安裝指令如下:
```
apt-get install libcurl4-openssl-dev   (CentOS系列可能是yum install curl-devel)
apt-get install -y rapidjson-dev
```

編譯時需要link curl 與thread
編譯指令如下:(以g++為範例，如使用其他方式編譯如CMAKE要額外設定參數)
```
g++ main.cpp stockDataHandler.cpp -o output -lcurl -lpthread
```

歷史資料來源使用台灣加權指數
台灣加權指數資料來源:
https://www.twse.com.tw/zh/page/trading/indices/MI_5MINS_HIST.html

## 執行範例:

input:

2

2021 8 4

2021 8 3


output:

78.9297

97.5605

## 備註
1. 因為需要發http request取得資料來源，必須確保對外網路可以正常連線
2. 查無資料的日期不會顯示資料在上面
3. 附有output上帶日期的版本，在編譯時加入define WITHDATE即可顯示 (-DWITHDATE)
4. 最高值與最低值的區間，因為output的資料是浮點數，可能有部分日期查詢資料區間有極小的誤差(0.001)，這邊暫時不處理這個部分，僅先用內建的浮點數相減