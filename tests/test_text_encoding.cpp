#include <cassert>
#include "text_encoding.h"
int main(){
assert(gbkToUtf8("\xcc\xda\xd1\xb6\xbf\xd8\xb9\xc9")==u8"腾讯控股");
assert(gbkToUtf8("\xb9\xf3\xd6\xdd\xc3\xa9\xcc\xa8")==u8"贵州茅台");
assert(gbkToUtf8("\xc4\xfe\xb5\xc2\xca\xb1\xb4\xfa")==u8"宁德时代");
assert(gbkToUtf8("\xbb\xc6\xbd\xf0\x45\x54\x46")==u8"黄金ETF");
assert(gbkToUtf8("\xb0\xa2\xc0\xef\xb0\xcd\xb0\xcd")==u8"阿里巴巴");
assert(gbkToUtf8("AAPL")=="AAPL");assert(gbkToUtf8(u8"腾讯控股")==u8"腾讯控股");std::string s=u8"腾讯控股";removeLastUtf8(s);assert(s==u8"腾讯控");assert(validUtf8(s));}
