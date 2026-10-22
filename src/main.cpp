#include <esp_wifi.h>
#include "power_policy.h"
#include "wifi_scan_policy.h"
#include "local_clock.h"
#include "boundary_feedback.h"
#include "quote_cache_policy.h"
#include "home_keys.h"
#include "quote_summary.h"
#include <M5Cardputer.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include <time.h>
#include "tencent_data.h"
#include "watchlist_utils.h"
#include "text_encoding.h"
#include "market_sessions.h"
#include "motion_deck.h"
#include "market_summary.h"
#include "quote_policy.h"
#ifndef STOCK_ENABLE_MOTION
#define STOCK_ENABLE_MOTION 1
#endif
constexpr bool motionFeatureAvailable=STOCK_ENABLE_MOTION;
const char* releaseVersion=motionFeatureAvailable?"v5.24 M":"v5.24 S";

// 公共固件不预置 Wi-Fi。首次启动会直接进入 Wi-Fi 扫描界面让用户输入。
// Public firmware ships with no preset Wi-Fi credentials — first boot will scan and prompt.
const char* defaultSSID = "";
const char* defaultPass = "";

Preferences prefs;

std::vector<String> stockCodes;
int stockCount = 0;

unsigned long lastFetchTime = 0;
unsigned long fetchInterval = 60000;
uint8_t screenBrightness = 100;
bool isConnected = false;
unsigned long lastReconnect=0;
bool hasQuoteFetch = false;
int64_t serverClock = 0;
unsigned long serverClockMillis = 0;
int64_t marketNow() {
    int64_t t=time(nullptr);
    return t>1700000000?t:serverClock?serverClock+(uint32_t)(millis()-serverClockMillis)/1000:0;
}
void syncHttpClock(HTTPClient& http) {
    int64_t stamp=parseHttpDate(http.header("Date").c_str());
    if(stamp>1700000000) {serverClock=stamp;serverClockMillis=millis();}
}
void prepareHttp(HTTPClient& http) {
    http.useHTTP10(true);http.setConnectTimeout(4000);http.setTimeout(5000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    const char* keys[]={"Date"};http.collectHeaders(keys,1);
    http.setUserAgent("Mozilla/5.0");
}
unsigned long lastQuoteSuccess = 0;
M5Canvas ui(&M5Cardputer.Display);
lgfx::LGFXBase* screen = &M5Cardputer.Display;
bool bufferedUI = false;
bool displayOff = false;
void applyBrightness() { if(!displayOff&&bufferedUI)ui.pushSprite(0,0); M5Cardputer.Display.setBrightness(displayOff ? 0 : (screenBrightness * 255 / 100)); }

enum AppState {
    STATE_HOME,
    STATE_CHART,
    STATE_EDIT,
    STATE_SETTINGS,
    STATE_WIFI_SCAN,
    STATE_WIFI_INPUT,
    STATE_PHONE_MANAGER,
    STATE_SEARCH,
    STATE_DELETE_CONFIRM
};

const char* chartTypeNames[] = {"5D", "D-K", "W-K", "M-K", "Y-K"};
bool lineStyle = false;

AppState currentState = STATE_HOME;
AppState searchReturnState=STATE_HOME,phoneReturnState=STATE_HOME;
ChartType currentChartType = CHART_5D;
int currentChartIdx = 0;
int homeScrollOffset = 0;
int chartZoomLevel = 300;
String editBuffer = "";

int deleteStockIdx=-1;
String deleteStockCode,deleteError;

MarketFeed marketFeed;
MarketPresence listedMarkets;
void updateListedMarkets(){listedMarkets=marketsIn(stockCodes);}
unsigned long marketFeedAttempt=0;
bool marketFeedOk=false;
bool motionEnabled=false;
bool homeKeysHeld=false;
BoundaryPulse boundaryPulse;
BoundaryAttemptLatch listBoundaryLatch,cardKeyBoundaryLatch,cardTiltBoundaryLatch;
unsigned long homeKeyMovedAt=0;
int motionSelected=0;
unsigned long motionMovedAt=0;
MotionDeck motionDeck;
String motionStatus;
void toggleMotion();
void moveMotionSelection(int direction);
void tickMotion();
void tickHomeKeys();
void tickBoundaryFeedback();
void showMotionHome();

int settingsCursor = 0;
int localTimeZone=0;
int wifiCursor = 0;
int wifiCount = 0;
bool wifiScanDone = false;
std::vector<String> wifiSSIDs;
std::vector<int> wifiRSSIs;
String wifiJoinSSID;
bool wifiNameInput=false;
String wifiInputBuffer = "";

struct StockInfo {
    String name;
    String price;
    String change;
    String changePercent;
    String exchangeCode;
    bool valid;
    bool fresh;
    unsigned long quoteFetchedAt=0,quoteAttemptAt=0;
    String quoteTime;
    String displayName;
    int64_t regularStart = 0, regularEnd = 0;
    PreQuote pre;
    unsigned long preAttempt = 0;
    bool preRequested = false;
    bool preError = false;
    String preErrorText;
};
std::vector<StockInfo> stocks;
String watchlistError;
int quoteScopeFirst=-1,quoteScopeEnd=-1;
uint32_t watchlistGeneration=0;
bool prefetchBusy=false;

ChartBar chartBars[300];
int chartCount = 0;
int chartLoadedIdx = -1;
ChartType chartLoadedPeriod = CHART_5D;
bool chartError = false;
String chartErrorText;
String chartPrimaryError,chartFallbackError;
bool chartFiveCloses = false;
unsigned long chartFetchedAt = 0;

String canonicalWatchlist(){String out;for(int i=0;i<stockCount;++i){if(i)out+=",";out+=stockCodes[i];}return out;}
bool loadStocksFromString(String str);
void fetchMinuteData();
void tickQuotePrefetch();
void tickLocalClock();
bool visibleQuotesReady(int first,int end);
void loadChartData(int idx);
void showHome();
void showChart();
void showEditScreen();
void showSettings();
void showWifiScan();
void showWifiInput();
void handleKeyboard();
void tickMarketFeed();
void beginDeleteStock();
void cancelDeleteStock();
void confirmDeleteStock();
void handleDeleteKeys(bool esc,bool enter);
void showDeleteConfirm();
void showSearch();
void leaveSearch();
void runStockSearch();
void addSearchHit();
void handleSearchKeys(const Keyboard_Class::KeysState& st,bool esc,bool enter,bool del,bool tab);
void tickPremarket();
void drawMessage(const String& title, const String& body);
void showPhoneManager();
void startPhoneManager();
void stopPhoneManager();
void tickPhoneManager();
void manualRefresh(){
    if(currentState!=STATE_HOME&&currentState!=STATE_CHART)return;
    for(int i=0;i<stockCount;++i)stocks[i].preAttempt=0;
    marketFeedAttempt=0;
    chartFetchedAt=millis()-60001UL;
    drawMessage("REFRESH","Updating quotes...");
    fetchMinuteData();lastFetchTime=millis();
    if(currentState==STATE_HOME)showHome();else showChart();
}

bool loadStocksFromString(String str) {
    std::vector<std::string> codes;std::string error;
    if(!parseSymbols(str.c_str(),codes,error)){watchlistError=error.c_str();return false;}
    size_t needed=codes.size()*(sizeof(StockInfo)+sizeof(String))+4096;
    // No fixed count limit. Keep enough heap for the 64KB JSON document and TLS.
    if(ESP.getFreeHeap()<needed+135000 || ESP.getMaxAllocHeap()<codes.size()*sizeof(StockInfo)+4096){watchlistError="Not enough free memory";return false;}
    std::vector<String> nextCodes;std::vector<StockInfo> nextStocks;
    nextCodes.reserve(codes.size());nextStocks.resize(codes.size());
    for(size_t i=0;i<codes.size();++i){nextCodes.emplace_back(codes[i].c_str());nextStocks[i]=StockInfo{};nextStocks[i].name="Loading...";}
    ++watchlistGeneration;stockCodes.swap(nextCodes);stocks.swap(nextStocks);stockCount=stocks.size();updateListedMarkets();
    chartLoadedIdx=-1;chartCount=0;quoteScopeFirst=-1;quoteScopeEnd=-1;
    homeScrollOffset=0;currentChartIdx=0;watchlistError="";return true;
}

#include "wifi_scan_runtime.h"
void resumeWifi(){
    WiFi.setAutoReconnect(true);
    String ssid=prefs.getString("wifi_ssid",defaultSSID),pass=prefs.getString("wifi_pass",defaultPass);
    if(ssid.length()&&WiFi.status()!=WL_CONNECTED&&(!lastReconnect||millis()-lastReconnect>=5000)){lastReconnect=millis();WiFi.begin(ssid.c_str(),pass.c_str());}
}
int batteryPercent=-1;unsigned long batterySampled=0;
void tickBattery(){
    if(batterySampled&&millis()-batterySampled<30000)return;
    batterySampled=millis();int level=M5Cardputer.Power.getBatteryLevel();
    batteryPercent=level>=0?max(0,min(100,level)):-1;
}

void setup() {
    Serial.begin(115200);
    delay(500);
    auto cfg = M5.config();
    cfg.internal_imu=motionFeatureAvailable;
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    ui.setColorDepth(8); // RGB332: 32,400 bytes instead of a 64,800-byte RGB565 frame.
    bufferedUI = ui.createSprite(240, 135) != nullptr;
    if (bufferedUI) screen = &ui;
    M5Cardputer.Display.setTextWrap(false);
    screen->setTextWrap(false);
    
    prefs.begin("stocks", false);
    localTimeZone=prefs.getInt("localZone",0);if(localTimeZone<0||localTimeZone>=LOCAL_ZONE_COUNT)localTimeZone=0;
    fetchInterval = prefs.getULong("interval", 60000);
    screenBrightness = prefs.getUChar("brightness", 60);
    if (screenBrightness < 20 || screenBrightness > 100) screenBrightness = 60;
    currentChartType = (ChartType)prefs.getInt("periodV3", 0);
    if ((int)currentChartType < 0 || currentChartType >= CHART_TYPE_MAX) currentChartType = CHART_5D;
    
    chartZoomLevel=currentChartType==CHART_5D?300:60;
    lineStyle = prefs.getBool("lineStyle", false);
    motionEnabled=motionFeatureAvailable&&prefs.getBool("motion",false)&&M5.Imu.isEnabled();
    applyBrightness();
    tickBattery();
    drawMessage("POCKET MARKET", "Starting Wi-Fi...");
    configTime(0, 0, "pool.ntp.org", "time.cloudflare.com");
    String savedStocks = prefs.getString("list", "sh000001,hk00700,usAAPL,usNVDA,usIBIT,sz159934");
    if(loadStocksFromString(savedStocks)){
        String canonical=canonicalWatchlist();
        if(canonical!=savedStocks){size_t written=prefs.putString("list",canonical);if(canonical.length()&&written!=canonical.length())Serial.println("Watchlist canonical save failed; normalized in RAM");}
    }
    
    String savedSSID = prefs.getString("wifi_ssid", defaultSSID);
    String savedPass = prefs.getString("wifi_pass", defaultPass);
    
    WiFi.mode(WIFI_STA);WiFi.setSleep(true);
    WiFi.begin(savedSSID.c_str(), savedPass.c_str());lastReconnect=millis();
    WiFi.setAutoReconnect(true);
    
    // Do not block keyboard/settings while waiting for Wi-Fi or Internet.
    isConnected=WiFi.status()==WL_CONNECTED;
    showHome();
}

void loop() {
    M5Cardputer.update();
    
    if(M5Cardputer.BtnA.wasPressed())manualRefresh();

    handleKeyboard();
    
    tickWifiScan();
    tickBattery();
    tickPhoneManager();
    tickMotion();
    tickHomeKeys();
    tickBoundaryFeedback();
    tickQuotePrefetch();
    tickLocalClock();
    if(!prefetchBusy&&!homeKeysHeld&&(!motionEnabled||currentState!=STATE_HOME||millis()-motionMovedAt>900))tickMarketFeed();

    if (currentState == STATE_EDIT) {
        static unsigned long lastBlink = 0;
        if (millis() - lastBlink > 500) {
            lastBlink = millis();
            showEditScreen();
        }
    } else if (currentState == STATE_WIFI_INPUT) {
        static unsigned long lastBlink = 0;
        if (millis() - lastBlink > 500) {
            lastBlink = millis();
            showWifiInput();
        }
    } else if(currentState==STATE_HOME || currentState==STATE_CHART) {
        isConnected = WiFi.status() == WL_CONNECTED;
        if (!isConnected && millis() - lastReconnect > 15000) {
            lastReconnect = millis();
            WiFi.reconnect();
        }
        int first=currentState==STATE_CHART?currentChartIdx:homeScrollOffset;
        int end=currentState==STATE_CHART?first+1:min(stockCount,first+3);
        if (isConnected && stockCount>0 && !prefetchBusy && !homeKeysHeld && millis()-homeKeyMovedAt>180 && (!motionEnabled||currentState!=STATE_HOME||millis()-motionMovedAt>900) && ((!visibleQuotesReady(first,end)&&(millis()-lastFetchTime>=max(5000UL,min(fetchInterval,15000UL))||!hasQuoteFetch||first!=quoteScopeFirst||end!=quoteScopeEnd)) || (currentState==STATE_CHART&&chartLoadedIdx!=currentChartIdx))) {
            fetchMinuteData();
            lastFetchTime = millis();
            hasQuoteFetch = true;
            if (currentState == STATE_HOME) showHome();
            else if (currentState == STATE_CHART) showChart();
        }
        if(!prefetchBusy&&!homeKeysHeld&&(!motionEnabled||currentState!=STATE_HOME||millis()-motionMovedAt>900))tickPremarket();
        static unsigned long lastStatusDraw = 0;
        if (millis() - lastStatusDraw >= 10000) {
            lastStatusDraw = millis();
            if (currentState == STATE_HOME) showHome();
            else if (currentState == STATE_CHART) showChart();
        }
    }
    delay(idleYieldMs(displayOff,motionEnabled&&currentState==STATE_HOME,currentState==STATE_PHONE_MANAGER));
}

#include "back_navigation.h"

void tickHomeKeys(){
    static HomeKeyRepeat repeat;
    if(currentState!=STATE_HOME||motionEnabled||displayOff){repeat.reset();listBoundaryLatch.reset();homeKeysHeld=false;return;}
    int direction=homeKeyDirection(M5Cardputer.Keyboard.keysState());homeKeysHeld=direction!=0;
    int step=repeat.update(direction,millis());
    if(direction)homeKeyMovedAt=millis(); // Let scrolling finish before synchronous network requests.
    if(!step)return;
    int next=cyclicListOffset(homeScrollOffset,step,stockCount);
    if((step<0&&next>homeScrollOffset)||(step>0&&next<homeScrollOffset))boundaryPulse.trigger(step,millis());
    if(next!=homeScrollOffset){homeScrollOffset=next;showHome();}
}

void tickBoundaryFeedback(){
    static bool wasActive=false;static uint32_t drawn=0;
    if(currentState!=STATE_HOME||displayOff){boundaryPulse.reset();wasActive=false;return;}
    bool active=boundaryPulse.active(millis());
    if((active&&millis()-drawn>=30)||(!active&&wasActive)){drawn=millis();showHome();}
    wasActive=active;
}

void handleKeyboard() {
    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        Keyboard_Class::KeysState st = M5Cardputer.Keyboard.keysState();
        
        bool enterPressed = st.enter;
        bool escPressed = st.esc;
        // Plain DEL key is reported as backspace; Fn+DEL is reported as del.
        bool backspacePressed = st.backspace || st.del;
        bool tabPressed = st.tab;
        
        for (char c : st.word) {
            if (c == 27 || c == '`') escPressed = true;
            if (c == '\r' || c == '\n') enterPressed = true;
            if (c == 8 || c == 127) backspacePressed = true;
            if (c == '\t') tabPressed = true;
        }

        if(escPressed){navigateBack();return;}

        if (currentState != STATE_EDIT && currentState != STATE_WIFI_INPUT && currentState != STATE_SEARCH) {
            for (char c : st.word) if (c == 'b' || c == 'B') {
                displayOff = !displayOff; applyBrightness(); return;
            }
        }

        if(currentState==STATE_DELETE_CONFIRM){
            handleDeleteKeys(escPressed,enterPressed);
            return; // D, DEL and all other keys cannot confirm a deletion.
        }
        if(currentState==STATE_SEARCH){handleSearchKeys(st,escPressed,enterPressed,backspacePressed,tabPressed);return;}
        if(currentState==STATE_PHONE_MANAGER){
            if(escPressed){stopPhoneManager();return;}
            return;
        }
        if(currentState==STATE_HOME||currentState==STATE_CHART){
            for(char c:st.word){
                if(currentState==STATE_CHART&&(c=='d'||c=='D')){beginDeleteStock();return;}
                if(motionFeatureAvailable&&currentState==STATE_HOME&&(c=='g'||c=='G')){toggleMotion();showHome();return;}
                if(c=='f'||c=='F'){searchReturnState=currentState;currentState=STATE_SEARCH;showSearch();return;}
                if(c=='u'||c=='U'){manualRefresh();return;}
                if(c=='w'||c=='W'){phoneReturnState=currentState;startPhoneManager();return;}
            }
        }

        if (currentState == STATE_EDIT) {
            if (escPressed) {
                currentState = STATE_HOME;
                showHome();
                return;
            } else if (enterPressed) {
                String previous=prefs.getString("list","");
                if(!loadStocksFromString(editBuffer)){showEditScreen();return;}
                String canonical=canonicalWatchlist();
                size_t written=prefs.putString("list",canonical);
                if(canonical.length() && written!=canonical.length()){
                    loadStocksFromString(previous);watchlistError="Storage full; list not saved";showEditScreen();return;
                }
                currentState = STATE_HOME;
                drawMessage("WATCHLIST", "Updating quotes...");
                fetchMinuteData();
                showHome();
                return;
            } else if (backspacePressed) {
                if (editBuffer.length() > 0) {
                    editBuffer.remove(editBuffer.length() - 1);
                }
            } else {
                for (char c : st.word) {
                    if (c >= 32 && c <= 126 && c != '`') {
                        editBuffer += c;
                    }
                }
            }
            if (currentState == STATE_EDIT) showEditScreen();
            return;
        } else if (currentState == STATE_SETTINGS) {
            if (escPressed) {
                currentState = STATE_HOME;
                resumeWifi();
                showHome();
                return;
            }
            for (char c : st.word) {
                if (c == ';' || c == ',') {
                    if (settingsCursor > 0) settingsCursor--;
                    showSettings();
                } else if (c == '.' || c == '/') {
                    if (settingsCursor < (motionFeatureAvailable?4:3)) settingsCursor++;
                    showSettings();
                }
            }
            if (enterPressed) {
                if (settingsCursor == 0) {
                    startWifiScan();
                } else if (settingsCursor == 1) {
                    if (fetchInterval == 5000) fetchInterval = 10000;
                    else if (fetchInterval == 10000) fetchInterval = 30000;
                    else if (fetchInterval == 30000) fetchInterval = 60000;
                    else if (fetchInterval == 60000) fetchInterval = 300000;
                    else fetchInterval = 5000;
                    prefs.putULong("interval", fetchInterval);
                    showSettings();
                } else if (settingsCursor == 2) {
                    screenBrightness += 20;
                    if (screenBrightness > 100) screenBrightness = 20;
                    prefs.putUChar("brightness", screenBrightness);
                    applyBrightness();
                    showSettings();
                } else if(settingsCursor==3){localTimeZone=(localTimeZone+1)%LOCAL_ZONE_COUNT;prefs.putInt("localZone",localTimeZone);showSettings();}
                else if(settingsCursor==4&&motionFeatureAvailable){toggleMotion();showSettings();}
            }
        } else if (currentState == STATE_WIFI_SCAN) {
            for(char c:st.word)if(c=='r'||c=='R'){startWifiScan();return;}
            if (escPressed) {
                cancelWifiScan();resumeWifi();
                currentState = STATE_SETTINGS;
                showSettings();
                return;
            }
            for (char c : st.word) {
                if (c == ';' || c == ',') {
                    if (wifiCursor > 0) wifiCursor--;
                    showWifiScan();
                } else if (c == '.' || c == '/') {
                    if (wifiCursor < wifiCount - 1) wifiCursor++;
                    showWifiScan();
                }
            }
            if (enterPressed && wifiCount > 0) {
                cancelWifiScan();
                currentState = STATE_WIFI_INPUT;
                wifiJoinSSID=wifiSSIDs[wifiCursor];wifiNameInput=wifiJoinSSID.length()==0;
                wifiInputBuffer = "";
                showWifiInput();
            }
        } else if (currentState == STATE_WIFI_INPUT) {
            if (escPressed) {
                currentState = STATE_WIFI_SCAN;
                showWifiScan();
                return;
            }
            if (enterPressed) {
                if(wifiNameInput){
                    if(!wifiInputBuffer.length()){showWifiInput();return;}
                    wifiJoinSSID=wifiInputBuffer;wifiInputBuffer="";wifiNameInput=false;showWifiInput();return;
                }
                String s = wifiJoinSSID;
                prefs.putString("wifi_ssid", s);
                prefs.putString("wifi_pass", wifiInputBuffer);
                WiFi.disconnect();
                WiFi.setAutoReconnect(true);
                WiFi.begin(s.c_str(), wifiInputBuffer.c_str());lastReconnect=millis();
                drawMessage("WI-FI", "Connecting...");
                int n = 0;
                while (WiFi.status() != WL_CONNECTED && n < 10) {
                    delay(500); n++;
                }
                isConnected = (WiFi.status() == WL_CONNECTED);
                currentState = STATE_HOME;
                if (isConnected) fetchMinuteData();
                showHome();
                return;
            }
            if (backspacePressed) {
                if (wifiInputBuffer.length() > 0) wifiInputBuffer.remove(wifiInputBuffer.length() - 1);
            } else {
                for (char c : st.word) {
                    if (c >= 32 && c <= 126 && c != '`' && (!wifiNameInput || wifiInputBuffer.length()<32)) wifiInputBuffer += c;
                }
            }
        } else if (currentState == STATE_HOME) {
            if(motionEnabled&&(st.left||st.right))return; // tickMotion handles tap/hold without duplicate steps.
            if(motionEnabled&&enterPressed&&stockCount>0){currentChartIdx=max(0,min(motionSelected,stockCount-1));currentState=STATE_CHART;loadChartData(currentChartIdx);showChart();return;}
            if (enterPressed) {
                currentState = STATE_EDIT;
                watchlistError="";
                editBuffer = prefs.getString("list", "sh000001,hk00700,usAAPL,usNVDA,usIBIT,sz159934");
                showEditScreen();
                return;
            } else if (tabPressed) {
                currentState = STATE_SETTINGS;
                settingsCursor = 0;
                showSettings();
                return;
            }
            for (char c : st.word) {
                if(motionEnabled&&(c==';'||c==','||c=='.'||c=='/'))continue; // polled by tickMotion, including key holds
                if(!motionEnabled&&(c==';'||c==','||c=='.'||c=='/'))continue; // polled tap/hold
                if (!motionEnabled && c >= '1' && c <= '3') {
                    int idx = homeScrollOffset+c-'1';
                    if (idx>=0 && idx < stockCount) {
                        currentState = STATE_CHART;
                        currentChartIdx = idx;
                        loadChartData(currentChartIdx);
                        showChart();
                    }
                }
            }
        } else if (currentState == STATE_CHART) {
            if (escPressed) {
                currentState = STATE_HOME;
                showHome();
                return;
            } else if (tabPressed) {
                currentChartType = (ChartType)((currentChartType + 1) % CHART_TYPE_MAX);
                prefs.putInt("periodV3", currentChartType);
                chartZoomLevel=currentChartType==CHART_5D?300:60;
                drawMessage("CHART", "Loading " + String(chartTypeNames[currentChartType]) + "...");
                loadChartData(currentChartIdx);
                showChart();
                return;
            }
            for (char c : st.word) {
                if (c == ' ') {
                    lineStyle = !lineStyle; prefs.putBool("lineStyle", lineStyle);
                    showChart(); return;
                }
                if (c == ';' || c == ',') {
                    currentChartIdx--;
                    if (currentChartIdx < 0) currentChartIdx = stockCount - 1;
                    loadChartData(currentChartIdx);
                    showChart();
                } else if (c == '.' || c == '/') {
                    currentChartIdx++;
                    if (currentChartIdx >= stockCount) currentChartIdx = 0;
                    loadChartData(currentChartIdx);
                    showChart();
                } else if (c == '-' || c == '_' || c == 'x' || c == 'X') {
                    chartZoomLevel = zoomWindow(chartZoomLevel,chartCount,false);
                    showChart();
                } else if (c == '=' || c == '+' || c == 'z' || c == 'Z') {
                    chartZoomLevel = zoomWindow(chartZoomLevel,chartCount,true);
                    showChart();
                }
            }
        }
    }
}

String yahooSymbol(const String& code) {
    if (code.startsWith("us")) { String t = code.substring(2); t.replace('.', '-'); return t; }
    if (code.startsWith("sh")) return code.substring(2) + ".SS";
    if (code.startsWith("sz")) return code.substring(2) + ".SZ";
    if (code.startsWith("hk")) { String t=code.substring(2); while(t.length()>4 && t[0]=='0') t.remove(0,1); return t+".HK"; }
    return "";
}
bool fetchYahooChart(int idx) {
    String ticker=yahooSymbol(stockCodes[idx]);if(!ticker.length())return false;
    WiFiClientSecure client;client.setInsecure();client.setHandshakeTimeout(5);client.setTimeout(5);
    HTTPClient http;
    String url="https://query1.finance.yahoo.com/v8/finance/chart/"+ticker+"?interval="+chartInterval(currentChartType)+"&range="+chartRange(currentChartType)+"&includePrePost=false";
    if(!http.begin(client,url)){chartErrorText="YF INIT";return false;}
    prepareHttp(http);int status=http.GET();syncHttpClock(http);
    if(status==HTTP_CODE_OK && http.getSize()<=180000){
        DynamicJsonDocument filter(2048);setChartFilter(filter);DynamicJsonDocument doc(49152);
        auto error=deserializeJson(doc,http.getStream(),DeserializationOption::Filter(filter));
        if(!error){chartCount=decodeChart(doc["chart"]["result"][0],currentChartType,chartBars,300);if(chartCount)chartErrorText="";else chartErrorText="YF EMPTY";}
        else chartErrorText="YF "+String(error.c_str());
    }else chartErrorText="YF HTTP "+String(status);
    http.end();return chartCount>0;
}
// Resolve a missing US exchange identifier without recursively loading a chart.
bool ensureChartExchange(int idx){
    if(!stockCodes[idx].startsWith("us")||stocks[idx].exchangeCode.length())return true;
    WiFiClient client;HTTPClient http;
    if(!http.begin(client,"http://qt.gtimg.cn/q="+stockCodes[idx]))return false;
    prepareHttp(http);int status=http.GET();syncHttpClock(http);bool ok=false;
    if(status==HTTP_CODE_OK){String payload=http.getString();SummaryQuote q;
        if(decodeSummary(payload.c_str(),summaryCode(stockCodes[idx].c_str()),q)){
            stocks[idx].exchangeCode=q.exchange.c_str();ok=true;
        }
    }
    http.end();return ok;
}
bool fetchTencentChartEndpoint(int idx,const char* base) {
    String code=stockCodes[idx];
    if(!ensureChartExchange(idx)){chartErrorText="QT need quote refresh";return false;}
    if(code.startsWith("us"))code="us"+stocks[idx].exchangeCode;
    String type=currentChartType==CHART_1W?"week":(currentChartType==CHART_1M||currentChartType==CHART_1Y)?"month":"day";
    String limit=currentChartType==CHART_5D?"5":"120";
    String url=String(base)+"?param="+code+","+type+",,,"+limit;
    WiFiClientSecure client;client.setInsecure();client.setHandshakeTimeout(5);client.setTimeout(5);HTTPClient http;
    if(!http.begin(client,url)){chartErrorText="QT INIT";return false;}
    prepareHttp(http);http.addHeader("Referer","https://gu.qq.com/");int status=http.GET();syncHttpClock(http);
    if(status==HTTP_CODE_OK&&http.getSize()<=180000){
        DynamicJsonDocument filter(2048);filter["code"]=true;
        filter["data"][code][type]=true;filter["data"][code]["qfq"+type]=true;
        static_assert(JSON_ARRAY_SIZE(1)==16,"ESP32 chart memory budget must be rechecked");
        DynamicJsonDocument doc(32768);
        if(!doc.capacity()){chartErrorText="QT LOW HEAP";http.end();return false;}
        auto error=deserializeJson(doc,http.getStream(),DeserializationOption::Filter(filter));
        if(!error&&doc["code"].as<int>()==0){
            JsonArrayConst rows=doc["data"][code][type].as<JsonArrayConst>();
            if(rows.isNull())rows=doc["data"][code]["qfq"+type].as<JsonArrayConst>();
            chartCount=decodeTencentChart(rows,currentChartType,chartBars,300);
            if(chartCount){chartFiveCloses=currentChartType==CHART_5D;chartErrorText="";}else chartErrorText="QT EMPTY";
        }else chartErrorText=error?"QT "+String(error.c_str()):"QT API "+String(doc["code"].as<int>());
    }else chartErrorText="QT HTTP "+String(status);
    http.end();return chartCount>0;
}
bool fetchTencentChart(int idx){
    if(fetchTencentChartEndpoint(idx,"https://web.ifzq.gtimg.cn/appstock/app/kline/kline"))return true;
    String firstError=chartErrorText;Serial.println("QT primary: "+firstError);
    if(fetchTencentChartEndpoint(idx,"https://proxy.finance.qq.com/ifzqgtimg/appstock/app/kline/kline"))return true;
    Serial.println("QT backup: "+chartErrorText);chartErrorText=firstError+" / "+chartErrorText;return false;
}
void loadChartData(int idx) {
    if(idx<0||idx>=stockCount)return;
    if(!chartError&&chartLoadedIdx==idx&&chartLoadedPeriod==currentChartType&&millis()-chartFetchedAt<60000)return;
    chartCount=0;chartFiveCloses=false;chartPrimaryError="";chartFallbackError="";chartLoadedIdx=idx;chartLoadedPeriod=currentChartType;chartError=true;
    if(WiFi.status()!=WL_CONNECTED){chartErrorText="WIFI OFF";return;}
    bool ok=false;
    if(currentChartType==CHART_5D){
        ok=fetchYahooChart(idx);if(!ok){chartFallbackError=chartErrorText;ok=fetchTencentChart(idx);if(!ok)chartPrimaryError=chartErrorText;}
    }else{
        ok=fetchTencentChart(idx);if(!ok){chartPrimaryError=chartErrorText;ok=fetchYahooChart(idx);if(!ok)chartFallbackError=chartErrorText;}
    }
    chartError=!ok;chartFetchedAt=millis();
    if(chartError)Serial.println("Chart: "+chartErrorText);
}

void applySummaryQuote(int i,const SummaryQuote&q){
    auto& stock=stocks[i];stock.fresh=true;stock.quoteTime=q.time.c_str();lastQuoteSuccess=millis();stock.quoteFetchedAt=millis();
    stock.name=gbkToUtf8(q.name.c_str()).c_str();
    if(!stockCodes[i].startsWith("us"))stock.displayName=stock.name;
    else if(!q.english.empty())stock.displayName=q.english.c_str();
    stock.exchangeCode=q.exchange.c_str();stock.price=String(strtod(q.price.c_str(),nullptr),3);
    stock.change=(strtod(q.change.c_str(),nullptr)>0?String("+"):String(""))+q.change.c_str();
    stock.changePercent=(strtod(q.percent.c_str(),nullptr)>0?String("+"):String(""))+q.percent.c_str();stock.valid=true;
}
bool visibleQuotesReady(int first,int end){
    for(int i=first;i<end;++i)if(!cachedQuoteReady(stocks[i].valid,stocks[i].quoteFetchedAt,millis(),fetchInterval))return false;
    return true;
}
void fetchMinuteData() {
    if (WiFi.status() != WL_CONNECTED) {
        isConnected = false;
        return;
    }
    
    if (stockCount == 0) return;
    int first=currentState==STATE_CHART?currentChartIdx:homeScrollOffset;
    int end=currentState==STATE_CHART?first+1:min(stockCount,first+3);
    for(int i=first;i<end;++i){stocks[i].fresh=false;stocks[i].quoteAttemptAt=millis();}
    quoteScopeFirst=first;quoteScopeEnd=end;
    
    WiFiClient client;
    HTTPClient http;
    http.setTimeout(5000);
    
    String summaryUrl = "http://qt.gtimg.cn/q=";
    for (int i = first; i < end; i++) {
        summaryUrl += String(summaryCode(stockCodes[i].c_str()).c_str());
        if (i < end - 1) summaryUrl += ",";
    }
    
    if (http.begin(client, summaryUrl)) {
        prepareHttp(http);
        int code = http.GET();
        syncHttpClock(http);
        if (code == HTTP_CODE_OK) {
            String payload = http.getString();
            const std::string raw=payload.c_str();
            for(int i=first;i<end;++i){
                SummaryQuote q;
                if(!decodeSummary(raw,summaryCode(stockCodes[i].c_str()),q))continue;
                applySummaryQuote(i,q);
            }
        }
        http.end();
    }
    
    isConnected = WiFi.status() == WL_CONNECTED;
    if (currentState == STATE_CHART) loadChartData(currentChartIdx);
}

void tickPremarket() {
    if (WiFi.status() != WL_CONNECTED || stockCount == 0) return;
    if (currentState != STATE_HOME && currentState != STATE_CHART) return;
    static unsigned long lastRequest = 0;
    if (millis() - lastRequest < 1500) return;
    const int first = currentState == STATE_CHART ? currentChartIdx : homeScrollOffset;
    const int end = currentState == STATE_CHART ? first + 1 : min(stockCount, first + 3);
    for (int i = first; i < end; ++i) {
        StockInfo& stock = stocks[i];
        if (!stockCodes[i].startsWith("us")||!preMarketTime(stockCodes[i].c_str(),marketNow())) continue;
        const unsigned long interval = stock.preError ? 180000UL : 60000UL;
        if (stock.preRequested && stock.preAttempt && millis() - stock.preAttempt < interval) continue;
        lastRequest = millis();
        stock.preAttempt = millis();
        stock.preRequested = true;
        stock.preError = true;
        String ticker = yahooSymbol(stockCodes[i]);
        bool safe = ticker.length() > 0 && ticker.length() <= 15;
        for (unsigned j = 0; j < ticker.length(); ++j) {
            char c = ticker[j];
            if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '^')) safe = false;
        }
        if (safe) {
            // Tencent's US minute response explicitly labels pre/post in pandata.tag.
            String code=premarketRequestCode(stockCodes[i].c_str(),stock.exchangeCode.c_str()).c_str();
            WiFiClientSecure client;client.setInsecure();client.setHandshakeTimeout(5);client.setTimeout(5);HTTPClient http;
            String url="https://web.ifzq.gtimg.cn/appstock/app/usMinute/query?code="+code;
            if(http.begin(client,url)) {
                prepareHttp(http);int status=http.GET();syncHttpClock(http);
                if(status==HTTP_CODE_OK&&http.getSize()<=150000){
                    DynamicJsonDocument filter(2048);filter["code"]=true;filter["data"][code]["pandata"]=true;
                    filter["data"][code]["qt"]["market"][0]=true;
                    DynamicJsonDocument doc(6144);auto error=deserializeJson(doc,http.getStream(),DeserializationOption::Filter(filter));
                    if(!error&&doc["code"].as<int>()==0&&doc["data"][code].is<JsonObject>()) {
                        auto result=doc["data"][code];
                        const char* marketLine=result["qt"]["market"][0]|"";
                        MarketFeed nextFeed;if(decodeMarketFeed(marketLine,nextFeed)&&freshMarketFeed(nextFeed,marketNow())){marketFeed=nextFeed;marketFeedOk=true;}
                        stock.pre=decodeTencentPre(result["pandata"]);
                        stock.regularStart=stock.pre.sessionEnd;stock.regularEnd=stock.regularStart?stock.regularStart+6*3600+30*60:0;
                        // China local server time is another fallback when no Date header/NTP is available.
                        if(!serverClock){const char* market=result["qt"]["market"][0]|"";int64_t t=parseCivil(market);if(t){serverClock=t-8*3600;serverClockMillis=millis();}}
                        stock.preError=false;stock.preErrorText=stock.pre.valid?"":"NO PRE SESSION";
                    }else stock.preErrorText=error?"QT "+String(error.c_str()):"QT NO DATA";
                }else stock.preErrorText="QT HTTP "+String(status);
                http.end();
            }else stock.preErrorText="QT INIT";
            if(stock.preError){Serial.println("Pre: "+stock.preErrorText);}
        }
        if (currentState == STATE_HOME) showHome(); else showChart();
        return; // At most one request between keyboard scans.
    }
}

#include "web_manager.h"
#include "ui_views.h"

#include "search_view.h"

#include "delete_stock.h"

#include "market_feed.h"

#include "motion_control.h"

#include "quote_prefetch.h"
