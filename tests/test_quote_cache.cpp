#include "quote_cache_policy.h"
#include <cassert>
#include <iostream>
int main(){assert(cachedQuoteReady(true,100,200,60000));assert(!cachedQuoteReady(false,100,200,60000));assert(!cachedQuoteReady(true,100,60100,60000));assert(cachedQuoteReady(true,0xfffffff0u,10,100));auto a=prefetchOrder(20,8,11,0);int expected[]={11,7,12,6,13,5};for(int i=0;i<6;++i)assert(a[i]==expected[i]);bool seen[20]={};for(int i:a){assert(i>=0&&i<20&&!(i>=8&&i<11)&&!seen[i]);seen[i]=true;}assert(a.size()==17);assert(prefetchOrder(3,0,3,0).empty());auto b=prefetchOrder(10,0,3,9);assert(b[0]==3&&b[1]==9&&b[2]==4);std::cout<<"PASS shared cache freshness, wraparound, neighbor priority, full-list coverage and boundaries\n";}
