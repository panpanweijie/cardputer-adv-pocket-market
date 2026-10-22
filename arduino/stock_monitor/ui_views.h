#include "quote_visual_policy.h"
#pragma once
// Fixed 240 x 135 geometry; bitmap font, no external assets or animation.
constexpr uint16_t BG = 0x0862, PANEL = 0x10C4, EDGE = 0x2127;
constexpr uint16_t INK = 0xEF7D, MUTED = 0x94B3, ACCENT = 0x5E7A;
constexpr uint16_t UP = 0xF9C7, DOWN = 0x07D0, PRE = 0xFD89;

void presentUI() { if (bufferedUI&&!displayOff) ui.pushSprite(0, 0); }
void textAt(const String& value, int x, int y, uint16_t color = INK,
            float scale = 1, int right = -1) {
    if(scale>1)screen->setFont(&fonts::Font2);else screen->setFont(&fonts::Font0);
    screen->setTextSize(1);
    screen->setTextColor(color);
    screen->setTextWrap(false);
    String out = value;
    if (right >= 0) x = right - screen->textWidth(out);
    screen->setCursor(x, y);
    screen->print(out);
}
String fitText(String s, int width, float scale = 1) {
    if(scale>1)screen->setFont(&fonts::Font2);else screen->setFont(&fonts::Font0); screen->setTextSize(1);
    while (s.length() && screen->textWidth(s) > width) s.remove(s.length() - 1);
    return s;
}
void numberAt(const String& value, int right, int y, int width, uint16_t color, float preferred = 1.5, int baseline = -1) {
    // Choose native bitmap fonts, never fractional scaling of the 5x7 glyphs.
    float scale=preferred>1?1.5f:1.0f;
    if(scale>1)screen->setFont(&fonts::Font2);else screen->setFont(&fonts::Font0);screen->setTextSize(1);
    if(screen->textWidth(value)>width){scale=1;screen->setFont(&fonts::Font0);}
    String out=screen->textWidth(value)>width?String("--"):value;
    if(baseline>=0){
        screen->setTextColor(color);screen->setTextWrap(false);
        screen->setTextDatum(lgfx::textdatum_t::baseline_right);
        screen->drawString(out,right,baseline);
        screen->setTextDatum(lgfx::textdatum_t::top_left);
    }else textAt(out,0,y,color,scale,right);
}
void footer(const String& hint) {
    screen->fillRect(0, 121, 240, 14, BG);
    screen->drawFastHLine(6, 120, 228, EDGE);
    textAt(hint, 6, 125, MUTED);
}
void frame(const String& title, const String& status = "") {
    screen->fillScreen(BG);
    textAt(title, 6, 5, ACCENT);
    textAt(status, 0, 5, MUTED, 1, 234);
    screen->drawFastHLine(6, 18, 228, EDGE);
}
void drawMessage(const String& title, const String& body) {
    frame(title);
    screen->fillRoundRect(6, 39, 228, 48, 5, PANEL);
    textAt(fitText(body, 212), 14, 59, INK);
    footer(String("POCKET MARKET ")+releaseVersion);
    presentUI();
}
String compactSymbol(int idx) {
    String code = stockCodes[idx];
    if (code.startsWith("us") || code.startsWith("hk") || code.startsWith("sh") || code.startsWith("sz")) code = code.substring(2);
    return code;
}

String signedPercent(double value) { return (value >= 0 ? "+" : "") + String(value, 2) + "%"; }
bool visiblePre(int idx){return validPreMarket(stockCodes[idx].c_str(),stocks[idx].pre,marketNow());}
String companyName(int idx) {
    String n=stocks[idx].displayName;
    if (!n.length()) {
        if(stockCodes[idx].startsWith("us")) return compactSymbol(idx);
        String m=stockCodes[idx].substring(0,2);m.toUpperCase();return m+" / "+compactSymbol(idx);
    }
    int comma=n.indexOf(','); if(comma>0) n=n.substring(0,comma);
    for(unsigned j=0;j<n.length();++j) if((uint8_t)n[j]>126 || (uint8_t)n[j]<32) return compactSymbol(idx);
    if(n.length()>11) {int space=n.indexOf(' ');if(space>0 && space<=11)n=n.substring(0,space);}
    return fitText(n,66);
}
void pill(const String& label,int y,bool positive,bool valid) {
    screen->fillRoundRect(173,y,61,15,2,valid?(positive?UP:DOWN):EDGE);
    numberAt(label,230,y+4,56,valid?uint16_t(0):INK,1);
}
constexpr uint16_t QUOTE_WARN=0xDCA8;
uint16_t quoteDotColor(int i,const MarketSession& session){
 const auto&s=stocks[i];auto view=quoteVisualState(s.valid,session.phase,quoteAge(session,s.quoteTime.c_str(),marketNow()));
 if(preMarketTime(stockCodes[i].c_str(),marketNow())&&s.preRequested&&s.preError)view.dot=QuoteDot::ATTENTION;
 return view.dot==QuoteDot::ATTENTION?QUOTE_WARN:uint16_t(0);
}
void quoteDot(int x,int y,uint16_t color){if(color)screen->fillCircle(x,y,2,color);}
bool tradingPrice(int i,const MarketSession& session){return stocks[i].valid&&activePhase(session.phase);}

void drawCompanyName(int idx,int x,int y,uint16_t dotColor=0) {
    if(!stockCodes[idx].startsWith("us") && stocks[idx].displayName.length()) {
        screen->setFont(&fonts::efontCN_14);screen->setTextSize(1);screen->setTextColor(INK);screen->setTextWrap(false);
        std::string label=stocks[idx].displayName.c_str();
        while(!label.empty()&&screen->textWidth(label.c_str())>(dotColor?63:70))removeLastUtf8(label);
        screen->setTextDatum(lgfx::textdatum_t::baseline_left);
        screen->drawString(label.c_str(),x,y);
        quoteDot(x+screen->textWidth(label.c_str())+4,y-5,dotColor);
        screen->setTextDatum(lgfx::textdatum_t::top_left);
        screen->setFont(&fonts::Font0);screen->setTextSize(1);
    } else textAt(companyName(idx),x,y,MUTED);
}
void smallChinese(const String& value,int x,int y,int width,uint16_t color=MUTED){
 screen->setFont(&fonts::efontCN_12);screen->setTextSize(1);screen->setTextColor(color);screen->setTextWrap(false);
 std::string label=value.c_str();while(!label.empty()&&screen->textWidth(label.c_str())>width)removeLastUtf8(label);
 screen->setCursor(x,y);screen->print(label.c_str());screen->setFont(&fonts::Font0);
}
String quoteStamp(int i){
 std::string digits;for(unsigned char c:std::string(stocks[i].quoteTime.c_str()))if(std::isdigit(c))digits+=c;
 if(digits.size()!=14)return "NO TIME";
 return String((digits.substr(4,2)+"/"+digits.substr(6,2)+" "+digits.substr(8,2)+":"+digits.substr(10,2)).c_str());
}
void drawLocalClock(){
    screen->fillRect(88,3,78,13,BG);
    String clock=localClockText(localTimeZone,marketNow()).c_str();
    screen->setFont(&fonts::Font0);screen->setTextSize(1);
    textAt(clock,127-screen->textWidth(clock)/2,5,MUTED);
}
void drawConnectionBattery(){
    uint16_t wifiColor=WiFi.status()==WL_CONNECTED?uint16_t(0x3DFF):UP;
    screen->drawLine(174,6,178,3,wifiColor);screen->drawFastHLine(178,3,7,wifiColor);screen->drawLine(184,3,188,6,wifiColor);
    screen->drawLine(177,8,180,6,wifiColor);screen->drawFastHLine(180,6,3,wifiColor);screen->drawLine(182,6,185,8,wifiColor);
    screen->drawLine(179,10,181,9,wifiColor);screen->drawLine(181,9,183,10,wifiColor);screen->fillCircle(181,12,1,wifiColor);
    if(WiFi.status()!=WL_CONNECTED){screen->drawLine(176,4,186,14,wifiColor);}
    uint16_t batteryColor=batteryPercent>=0&&batteryPercent<20?UP:MUTED;
    screen->drawRoundRect(197,3,37,12,2,batteryColor);screen->fillRect(234,7,2,4,batteryColor);
    String level=batteryPercent>=0?String(batteryPercent)+"%":"--";
    screen->setFont(&fonts::Font0);screen->setTextSize(1);
    textAt(level,215-screen->textWidth(level)/2,5,batteryColor);
}
void homeHeader(const String& title){
    frame(title);
    if(boundaryPulse.active(millis())){screen->fillRoundRect(4,3,81,12,3,ACCENT);textAt(title,6,5,BG);}
    drawLocalClock();drawConnectionBattery();
}
void tickLocalClock(){
    static int64_t lastSecond=-1;
    if(currentState!=STATE_HOME||displayOff){lastSecond=-1;return;}
    int64_t now=marketNow();if(now==lastSecond)return;lastSecond=now;
    drawLocalClock();
    if(bufferedUI){M5Cardputer.Display.setClipRect(88,3,78,13);ui.pushSprite(0,0);M5Cardputer.Display.clearClipRect();}
}
void showHome() {
    if(motionFeatureAvailable&&motionEnabled&&stockCount>0){showMotionHome();return;}
    homeHeader(fitText("LIST "+String(stockCount?homeScrollOffset+1:0)+"/"+String(stockCount),78));
    for(int row=0;row<3 && homeScrollOffset+row<stockCount;++row) {
        int i=homeScrollOffset+row,y=22+row*32;const auto&s=stocks[i];bool pre=preMarketTime(stockCodes[i].c_str(),marketNow()),hasPre=visiblePre(i);
        screen->fillRoundRect(6,y,228,31,3,PANEL);
        auto session=viewSession(stockCodes[i].c_str(),marketNow(),marketFeed);
        
        textAt(String(row+1),11,y+5,MUTED);
        if (!stockCodes[i].startsWith("us") && s.displayName.length()) {
            drawCompanyName(i,21,y+15,quoteDotColor(i,session));
            textAt(compactSymbol(i),21,y+21,MUTED);
        } else {
            uint16_t dotColor=quoteDotColor(i,session);
            String symbol=fitText(compactSymbol(i),dotColor?59:66,1.5);
            screen->setFont(&fonts::Font2);screen->setTextSize(1);screen->setTextColor(INK);
            screen->setTextDatum(lgfx::textdatum_t::baseline_left);screen->drawString(symbol,23,y+15);
            quoteDot(23+screen->textWidth(symbol)+4,y+10,dotColor);
            screen->setTextDatum(lgfx::textdatum_t::top_left);
            textAt(companyName(i),23,y+21,MUTED);
        }
        numberAt(s.valid?s.price:"--",166,0,74,tradingPrice(i,session)?INK:MUTED,1.5,y+15);
        pill(s.valid?s.changePercent+"%":"--",y+1,s.change.toFloat()>=0,s.valid);
        if(pre) {
            String price=hasPre?String(s.pre.price,3):String("--");
            numberAt(price,166,y+21,74,MUTED,1);
            numberAt(hasPre&&s.pre.hasPercent?signedPercent(s.pre.percent):"--",230,y+21,61,s.pre.percent>=0?UP:DOWN,1);
        }
    }
    if(!stockCount) textAt("ENTER to add stock symbols",12,55,MUTED);
    if(boundaryPulse.active(millis())){
        int edgeY=boundaryPulse.direction<0?20:117;
        screen->fillRoundRect(10,edgeY,220,2,1,ACCENT);
    }
    footer("1-3 detail ;/. scroll U refresh");presentUI();
}
#include "motion_view.h"
String chartDate(int64_t timestamp) {
    time_t epoch=(time_t)timestamp;struct tm d;gmtime_r(&epoch,&d);char out[12];
    strftime(out,sizeof(out),currentChartType==CHART_5D||currentChartType==CHART_1D?"%m/%d":"%Y",&d);
    return String(out);
}
void plotChart(int idx,int x,int y,int w,int h) {
    bool line=currentChartType==CHART_5D||lineStyle;
    if(chartLoadedIdx!=idx||chartLoadedPeriod!=currentChartType||chartCount<(line?2:1)) {
        textAt("Press U to refresh",x+(w-108)/2,y+h/2-4,MUTED);
        return;
    }
    int first=max(0,chartCount-chartZoomLevel),visible=chartCount-first;
    float low=1e30f,high=-1e30f;
    for(int i=first;i<chartCount;++i) {
        low=min(low,line?chartBars[i].close:chartBars[i].low);
        high=max(high,line?chartBars[i].close:chartBars[i].high);
    }
    if(low<=0||!std::isfinite(low)||!std::isfinite(high)) return;
    float pad=max((high-low)*.08f,max(high*.0005f,.005f));low-=pad;high+=pad;
    auto yOf=[&](float p){return y+h-1-(int)((p-low)/(high-low)*(h-1));};
    for(int j=0;j<=2;++j)screen->drawFastHLine(x,y+j*(h-1)/2,w,EDGE);
    auto axisLabel=[](float p)->String {
        if(p>=1000000)return String(p/1000000,1)+"M";
        if(p>=10000)return String(p/1000,1)+"K";
        if(p>=100)return String(p,0);
        return String(p,p>=1?1:3);
    };
    numberAt(axisLabel(high),39,y,38,MUTED,1);
    numberAt(axisLabel(low),39,y+h-8,38,MUTED,1);
    uint16_t trend=chartBars[chartCount-1].close>=chartBars[first].close?UP:DOWN;
    int px=0,py=0;
    for(int i=first;i<chartCount;++i) {
        const auto&c=chartBars[i];int xx=x+(i-first)*(w-6)/max(1,visible-1);
        if(line){int yy=yOf(c.close);if(i>first)screen->drawLine(px,py,xx,yy,trend);px=xx;py=yy;}
        else{int cw=max(1,min(5,w/max(1,visible)-1));uint16_t color=c.close>=c.open?UP:DOWN;
            screen->drawLine(xx+cw/2,yOf(c.high),xx+cw/2,yOf(c.low),color);
            screen->fillRect(xx,min(yOf(c.open),yOf(c.close)),cw,max(1,abs(yOf(c.open)-yOf(c.close))),color);}
    }
    if(line) screen->fillCircle(px,py,2,trend);
    textAt(chartDate(chartBars[first].timestamp),x,111,MUTED,1);
    String zoom=String(min(chartZoomLevel,chartCount))+"/"+String(chartCount);
    textAt(chartFiveCloses?"5 close":zoom,108,111,MUTED,1);
    textAt(chartDate(chartBars[chartCount-1].timestamp),0,111,MUTED,1,234);
}
void showChart() {
    if(!stockCount){showHome();return;}
    int i=currentChartIdx;const auto&s=stocks[i];bool pre=preMarketTime(stockCodes[i].c_str(),marketNow()),hasPre=visiblePre(i);
    auto session=viewSession(stockCodes[i].c_str(),marketNow(),marketFeed);
    screen->fillScreen(BG);
    uint16_t dotColor=quoteDotColor(i,session);
    if(stockCodes[i].startsWith("hk")||stockCodes[i].startsWith("sh")||stockCodes[i].startsWith("sz")){
        String title=s.displayName.length()?s.displayName:s.name;if(!title.length())title="加载中";
        screen->setFont(&fonts::efontCN_12);screen->setTextSize(1);
        std::string label=title.c_str();while(!label.empty()&&screen->textWidth(label.c_str())>(dotColor?65:72))removeLastUtf8(label);
        int width=screen->textWidth(label.c_str());smallChinese(label.c_str(),6,5,72,INK);quoteDot(6+width+4,10,dotColor);
    }else{
        String title=fitText(compactSymbol(i),dotColor?65:72,1.5);textAt(title,6,4,INK,1.5);quoteDot(6+screen->textWidth(title)+4,10,dotColor);
    }
    String price=s.valid?s.price:"--";
    float priceScale=1.5f;screen->setFont(&fonts::Font2);screen->setTextSize(1);
    if(screen->textWidth(price)>80){priceScale=1;screen->setFont(&fonts::Font0);screen->setTextSize(1);}
    if(screen->textWidth(price)>80)price="--";
    textAt(price,120-screen->textWidth(price)/2,4,tradingPrice(i,session)?INK:MUTED,priceScale);
    numberAt(s.valid?s.changePercent+"%":"--",234,6,64,s.change.toFloat()>=0?UP:DOWN,1);
    String state=String(marketLabel(session.market))+" "+priceStage(session);
    auto age=quoteAge(session,s.quoteTime.c_str(),marketNow());
    if(WiFi.status()!=WL_CONNECTED)state+="·离线";
    
    
    if(pre){
        smallChinese("美股 收盘",6,22,62,MUTED);
        textAt("PRE",72,25,PRE);
        String prePrice=hasPre?String(s.pre.price,3):String("--");
        numberAt(prePrice,166,25,74,MUTED,1);
        numberAt(hasPre&&s.pre.hasPercent?signedPercent(s.pre.percent):"--",234,25,61,s.pre.percent>=0?UP:DOWN,1);
    }else{
        smallChinese(state,6,22,100,MUTED);
        textAt(quoteStamp(i),0,26,MUTED,1,234);
    }
    int tabsY=38;
    for(int j=0;j<5;++j){int x=6+j*46;screen->fillRoundRect(x,tabsY,43,15,3,j==(int)currentChartType?INK:PANEL);
        textAt(chartTypeNames[j],x+(j?9:12),tabsY+4,j==(int)currentChartType?BG:MUTED);}
    plotChart(i,44,60,190,45);
    footer("D del U refresh Z/X SPC TAB");presentUI();
}

void showSettings() {
    frame("SETTINGS",releaseVersion);
    const char* labels[] = {"Wi-Fi network", "Quote refresh", "Brightness", "Time zone", "Motion scroll"};
    for (int i = 0; i < (motionFeatureAvailable?5:4); ++i) {
        int y = 23 + i * 18;
        screen->fillRoundRect(6, y, 228, 17, 3, i == settingsCursor ? EDGE : PANEL);
        textAt(labels[i], 14, y + 5, i == settingsCursor ? ACCENT : INK);
        String value=i==0?">":i==1?String(fetchInterval/1000)+"s":i==2?String(screenBrightness)+"%":i==3?String(localZoneName(localTimeZone)):motionStatus.length()?"N/A":motionEnabled?"ON":"OFF";
        textAt(value, 0, y + 5, MUTED, 1, 225);
    }
    footer("` back  ;/. move  ENTER select");
    presentUI();
}
void showWifiScan() {
    frame("WI-FI NETWORKS", String(wifiCount)+(wifiScanDone?" found":wifiScanPass>1?" retry":" scanning"));
    int start = max(0, wifiCursor - 3);
    for (int i = start; i < min(wifiCount, start + 4); ++i) {
        int y = 24 + (i - start) * 23;
        screen->fillRoundRect(6, y, 228, 20, 3, i == wifiCursor ? EDGE : PANEL);
        screen->setFont(&fonts::efontCN_14);screen->setTextSize(1);screen->setTextWrap(false);
        std::string label=wifiSSIDs[i].length()?wifiSSIDs[i].c_str():"隐藏网络";
        while(!label.empty()&&screen->textWidth(label.c_str())>148)removeLastUtf8(label);
        screen->setTextColor(i==wifiCursor?ACCENT:INK);screen->setCursor(12,y+3);screen->print(label.c_str());
        textAt(String(wifiRSSIs[i])+"dBm",0,y+6,MUTED,1,230);
    }
    if (!wifiCount) textAt(wifiScanDone ? fitText(wifiScanError,216) : "Scanning...", 12, 55, MUTED);
    footer("R rescan ;/. move ENTER join");
    presentUI();
}
void inputPanel(const String& title, const String& hint, String value, bool secret, bool cnHint=false) {
    frame(title);
    if(cnHint){screen->setFont(&fonts::efontCN_14);screen->setTextSize(1);screen->setTextColor(MUTED);
        std::string label=hint.c_str();while(!label.empty()&&screen->textWidth(label.c_str())>224)removeLastUtf8(label);screen->setCursor(8,24);screen->print(label.c_str());
    }else textAt(fitText(hint, 224), 8, 27, MUTED);
    screen->fillRoundRect(6, 44, 228, 58, 4, PANEL);
    if (secret) { String mask; for (unsigned i = 0; i < value.length(); ++i) mask += '*'; value = mask; }
    // Show the edit cursor and the tail of a dynamic-length watchlist.
    if (value.length() > 102) value = value.substring(value.length() - 102);
    if ((millis() / 500) % 2 == 0) value += '_';
    for (int line = 0; line < 3; ++line) textAt(value.substring(line * 34, (line + 1) * 34), 12, 53 + line * 14, INK);
    footer("ENTER save/connect  ` cancel");
    presentUI();
}
void showWifiInput() {
    inputPanel(wifiNameInput?"WI-FI NAME":"WI-FI PASSWORD",wifiNameInput?"Hidden network: enter SSID":wifiJoinSSID,wifiInputBuffer,!wifiNameInput,!wifiNameInput);
}
void showEditScreen() {
    inputPanel("WATCHLIST "+String(stockCount),watchlistError.length()?watchlistError:"usAAPL,hk00700,...",editBuffer,false);
}

void showPhoneManager(){
    screen->fillScreen(BG);
    textAt("PHONE WATCHLIST",6,5,ACCENT);
    if(WiFi.status()!=WL_CONNECTED){
        textAt("Connect Wi-Fi first",8,44,INK);
        textAt("Back, then TAB > Wi-Fi",8,62,MUTED);
        textAt("` back  B screen",8,108,MUTED);
        presentUI();return;
    }
    String ip=WiFi.localIP().toString();String url="http://"+ip+"/";
    // Version 2: 25 modules at 3 pixels plus 4-module white margins = 99px.
    screen->qrcode(url.c_str(),5,25,99,2,true);
    textAt("Scan to edit",112,29,INK);
    textAt("Same Wi-Fi",112,46,MUTED);
    textAt(ip,112,63,ACCENT);
    textAt("Save on phone",112,80,INK);
    textAt(fitText(phoneSavedStatus,120),112,97,ACCENT);
    textAt("` back B screen",112,116,MUTED);
    presentUI();
}

void showDeleteConfirm(){
 frame("REMOVE STOCK?");
 screen->fillRoundRect(6,29,228,54,3,PANEL);
 if(deleteStockIdx>=0&&deleteStockIdx<stockCount){
  String name=stocks[deleteStockIdx].displayName;
  if(!name.length())name=compactSymbol(deleteStockIdx);
  screen->setFont(&fonts::efontCN_14);screen->setTextSize(1);screen->setTextColor(INK);screen->setTextWrap(false);
  std::string label=name.c_str();while(!label.empty()&&screen->textWidth(label.c_str())>208)removeLastUtf8(label);
  screen->setCursor(14,38);screen->print(label.c_str());
  textAt(stockCodes[deleteStockIdx],14,65,MUTED);
 }
 textAt(deleteError.length()?fitText(deleteError,224):"Remove from watchlist?",8,99,deleteError.length()?UP:MUTED);
 footer("ENTER confirm  ESC cancel");presentUI();
}
