#pragma once
#include <cstdint>
#include <algorithm>
inline uint32_t backgroundRefreshMs(uint32_t interval){return std::max<uint32_t>(60000,interval);}
inline uint32_t idleYieldMs(bool off,bool motion,bool web){return web?2:off?20:motion?2:10;}
