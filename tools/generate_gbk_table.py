from pathlib import Path
root=Path(__file__).resolve().parents[1]
values=[]
for lead in range(0x81,0xff):
    for trail in list(range(0x40,0x7f))+list(range(0x80,0xff)):
        try:
            text=bytes([lead,trail]).decode('gbk')
            code=ord(text) if len(text)==1 and ord(text)<=0xffff else 0
        except UnicodeDecodeError: code=0
        values.append(code)
lines=['#pragma once','#include <cstdint>','#ifdef ARDUINO','#include <pgmspace.h>','#define GBK_STORAGE PROGMEM','#else','#define GBK_STORAGE','#endif','// Generated from Python GBK codec; 126 lead bytes x 190 legal trail bytes.','static const uint16_t GBK_TO_UNICODE[23940] GBK_STORAGE = {']
for i in range(0,len(values),16):lines.append(','.join('0x%04X'%x for x in values[i:i+16])+',')
lines+=['};','#undef GBK_STORAGE','']
(root/'src/gbk_table.h').write_text('\n'.join(lines))
print('GBK mapping generated:',len(values)*2,'bytes in flash')
