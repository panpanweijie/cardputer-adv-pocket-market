#pragma once
#include <ArduinoJson.h>
#include <cmath>
#include <cstdint>

struct PreQuote {
    double price = 0;
    double percent = 0;
    int64_t timestamp = 0;
    int64_t sessionStart = 0;
    int64_t sessionEnd = 0;
    int32_t offset = 0;
    bool valid = false;
    bool hasPercent = false;
};

// Only accept trades in the provider's explicit pre-market session.
// Never infer pre-market from regularMarketPrice, fulldayPrice, or post-market bars.
inline PreQuote decodePreQuote(JsonVariantConst result) {
    PreQuote out;
    auto meta = result["meta"];
    auto pre = meta["currentTradingPeriod"]["pre"];
    if (!meta["hasPrePostMarketData"].as<bool>()) return out;
    out.sessionStart = pre["start"].as<int64_t>();
    out.sessionEnd = pre["end"].as<int64_t>();
    out.offset = pre["gmtoffset"].as<int32_t>();
    if (out.sessionStart <= 0 || out.sessionEnd <= out.sessionStart) return out;
    auto timestamps = result["timestamp"].as<JsonArrayConst>();
    auto closes = result["indicators"]["quote"][0]["close"].as<JsonArrayConst>();
    const size_t count = timestamps.size() < closes.size() ? timestamps.size() : closes.size();
    for (size_t i = 0; i < count; ++i) {
        if (!timestamps[i].is<int64_t>() || closes[i].isNull()) continue;
        const int64_t t = timestamps[i].as<int64_t>();
        const double p = closes[i].as<double>();
        if (t < out.sessionStart || t >= out.sessionEnd || t <= out.timestamp || !std::isfinite(p) || p <= 0) continue;
        out.price = p;
        out.timestamp = t;
        out.valid = true;
    }
    double base = meta["chartPreviousClose"] | 0.0;
    if (base <= 0) base = meta["previousClose"] | 0.0;
    if (out.valid && std::isfinite(base) && base > 0) {
        out.percent = (out.price / base - 1.0) * 100.0;
        out.hasPercent = true;
    }
    return out;
}

inline void setPreFilter(JsonDocument& filter) {
    auto r = filter["chart"]["result"][0];
    r["meta"]["hasPrePostMarketData"] = true;
    r["meta"]["currentTradingPeriod"]["pre"] = true;
    r["meta"]["currentTradingPeriod"]["regular"] = true;
    r["meta"]["shortName"] = true;
    r["meta"]["chartPreviousClose"] = true;
    r["meta"]["previousClose"] = true;
    r["timestamp"][0] = true;
    r["indicators"]["quote"][0]["close"][0] = true;
}
