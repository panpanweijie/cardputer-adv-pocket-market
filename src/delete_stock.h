#pragma once
// Persist before erasing in-memory data; the quote cache for other stocks is retained.
void beginDeleteStock(){
 if(currentState!=STATE_CHART||currentChartIdx<0||currentChartIdx>=stockCount)return;
 deleteStockIdx=currentChartIdx;deleteStockCode=stockCodes[deleteStockIdx];deleteError="";
 currentState=STATE_DELETE_CONFIRM;showDeleteConfirm();
}
void cancelDeleteStock(){
 currentState=STATE_CHART;deleteStockIdx=-1;deleteStockCode="";deleteError="";showChart();
}
void confirmDeleteStock(){
 if(deleteStockIdx<0||deleteStockIdx>=stockCount||stockCodes[deleteStockIdx]!=deleteStockCode){
  deleteError="List changed. ESC cancel";showDeleteConfirm();return;
 }
 String next;size_t bytes=0;for(int i=0;i<stockCount;++i)if(i!=deleteStockIdx)bytes+=stockCodes[i].length()+1;
 if(bytes&&!next.reserve(bytes)){deleteError="Low memory. ESC cancel";showDeleteConfirm();return;}
 for(int i=0;i<stockCount;++i)if(i!=deleteStockIdx){if(next.length())next+=',';next+=stockCodes[i];}
 size_t written=prefs.putString("list",next);
 // An empty NVS string legitimately returns zero; verify it separately.
 if((next.length()&&written!=next.length())||(!next.length()&&prefs.getString("list","__missing__")!="")){
  deleteError="Save failed. ENT retry";showDeleteConfirm();return;
 }
 ++watchlistGeneration;stockCodes.erase(stockCodes.begin()+deleteStockIdx);stocks.erase(stocks.begin()+deleteStockIdx);stockCount=stocks.size();updateListedMarkets();
 homeScrollOffset=min(homeScrollOffset,max(0,stockCount-3));currentChartIdx=min(deleteStockIdx,max(0,stockCount-1));
 chartLoadedIdx=-1;chartCount=0;quoteScopeFirst=-1;quoteScopeEnd=-1;
 deleteStockIdx=-1;deleteStockCode="";deleteError="";currentState=STATE_HOME;showHome();
}

void handleDeleteKeys(bool esc,bool enter){
 if(esc)cancelDeleteStock();else if(enter)confirmDeleteStock();
}
