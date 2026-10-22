#pragma once
#include "market_sessions.h"
enum class QuoteDot {NONE,TRADING,ATTENTION};
struct QuoteVisualState{bool bright=false;QuoteDot dot=QuoteDot::NONE;};
inline QuoteVisualState quoteVisualState(bool valid,TradingPhase phase,QuoteAge age){
 QuoteVisualState out;out.bright=valid&&activePhase(phase);
 if(out.bright&&age!=QuoteAge::OK)out.dot=QuoteDot::ATTENTION;
 return out;
}
