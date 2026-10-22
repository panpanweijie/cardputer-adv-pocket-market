#pragma once
// Isolated experimental renderer: one RGB332 frame, no texture assets or extra sprites.
void deckClip(){screen->setClipRect(0,35,240,84);}
void deckName(const String& text,int x,int y,int width,bool large,uint16_t dotColor=0){
 if(large)screen->setFont(&fonts::efontCN_14);else screen->setFont(&fonts::efontCN_12);
 screen->setTextSize(1);screen->setTextColor(INK);screen->setTextWrap(false);
 std::string out=text.c_str();while(!out.empty()&&screen->textWidth(out.c_str())>width-(dotColor?7:0))removeLastUtf8(out);
 screen->setCursor(x,y);screen->print(out.c_str());quoteDot(x+screen->textWidth(out.c_str())+4,y+(large?7:6),dotColor);
}
void drawDeckCard(const DeckCard& c){
 int x=c.x+boundaryPulse.offset(millis()),y=c.y,w=c.w,h=c.h;const auto& item=stocks[c.index];bool center=std::fabs(c.distance)<.35f;
 auto marketSession=viewSession(stockCodes[c.index].c_str(),marketNow(),marketFeed);
 uint16_t dotColor=quoteDotColor(c.index,marketSession);
 bool us=stockCodes[c.index].startsWith("us");
 bool selected=c.index==motionDeck.selected(stockCount);uint16_t face=selected?uint16_t(0x1926):PANEL;
 screen->fillRoundRect(x+3,y+3,w,h,6,0x0000);
 screen->fillRoundRect(x,y,w,h,6,face);screen->drawRoundRect(x,y,w,h,6,selected?ACCENT:EDGE);
 screen->drawFastHLine(x+9,y+3,w-18,selected?ACCENT:EDGE);
 if(selected&&boundaryPulse.active(millis())){int edgeX=boundaryPulse.direction<0?x:x+w-2;screen->fillRoundRect(edgeX,y+7,2,h-14,1,INK);}
 int left=max(0,x+4),top=max(35,y+4),right=min(240,x+w-4),bottom=min(119,y+h-4);
 if(right<=left||bottom<=top)return;
 screen->setClipRect(left,top,right-left,bottom-top);
 if(center){
  auto session=viewSession(stockCodes[c.index].c_str(),marketNow(),marketFeed);bool pre=preMarketTime(stockCodes[c.index].c_str(),marketNow()),hasPre=visiblePre(c.index);
  deckName(us?compactSymbol(c.index):(item.displayName.length()?item.displayName:compactSymbol(c.index)),x+8,y+7,w-16,true,dotColor);
  textAt(us?fitText(companyName(c.index),w-86):compactSymbol(c.index),x+8,y+26,MUTED);
  screen->fillRoundRect(x+w-69,y+23,61,15,2,item.valid?(item.change.toFloat()>=0?UP:DOWN):EDGE);
  numberAt(item.valid?item.changePercent+"%":"--",x+w-12,y+27,53,item.valid?uint16_t(0):INK,1);
  String price=item.valid?item.price:"--";screen->setFont(&fonts::Font4);screen->setTextSize(1);
  if(screen->textWidth(price)>w-16)screen->setFont(&fonts::Font2);
  if(screen->textWidth(price)>w-16)price="--";
  screen->setTextColor(tradingPrice(c.index,session)?INK:MUTED);screen->setCursor(x+8,y+39);screen->print(price);
  if(h>=79&&pre){
   uint16_t preColor=hasPre&&item.pre.hasPercent?(item.pre.percent>=0?UP:DOWN):MUTED;
   textAt("PRE",x+8,y+68,MUTED);
   String prePrice=hasPre?String(item.pre.price,3):String("--");
   numberAt(prePrice,x+w-70,y+68,w-104,preColor,1);
   numberAt(hasPre&&item.pre.hasPercent?signedPercent(item.pre.percent):"--",x+w-12,y+68,53,preColor,1);
  }else if(h>=79)textAt(quoteStamp(c.index),x+8,y+68,MUTED);
 }else if(std::fabs(c.distance)<.85f){
  deckName(us?compactSymbol(c.index):(item.displayName.length()?item.displayName:compactSymbol(c.index)),x+8,y+8,w-16,false,dotColor);
  textAt(fitText(us?companyName(c.index):compactSymbol(c.index),w-16),x+8,y+26,MUTED);
  numberAt(item.valid?item.price:"--",x+w-8,y+40,w-16,tradingPrice(c.index,marketSession)?INK:MUTED,1.5);
 }else{
  int tx=c.distance<0?max(x+8,8):min(x+w-44,196);
  String title=fitText(compactSymbol(c.index),dotColor?33:40);
  textAt(title,tx,y+16,MUTED);
  quoteDot(tx+screen->textWidth(title)+4,y+19,dotColor);
  textAt(us?fitText(companyName(c.index),40):(item.valid?item.price:"--"),tx,y+34,!us&&tradingPrice(c.index,marketSession)?INK:MUTED);
 }
 deckClip();
}
void showMotionHome(){
 motionSelected=motionDeck.selected(stockCount);homeScrollOffset=max(0,min(motionSelected-1,stockCount-3));
 homeHeader(fitText("FOCUS "+String(motionSelected+1)+"/"+String(stockCount),78));
 auto session=viewSession(stockCodes[motionSelected].c_str(),marketNow(),marketFeed);
 String status=String(marketLabel(session.market))+" "+priceStage(session);
 smallChinese(status,6,20,100,MUTED);
 deckClip();DeckCard cards[5];int count=deckCards(motionDeck.position,stockCount,cards);
 for(int i=0;i<count;++i)drawDeckCard(cards[i]);
 screen->clearClipRect();
 footer("ENTER detail  ESC back");presentUI();
}
