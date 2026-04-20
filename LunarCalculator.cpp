#include "LunarCalculator.h"
#include <cmath>

// ============================================================
//  Julian Day Number from a UTC QDateTime
//
//  Formula: Meeus, "Astronomical Algorithms" Chapter 7.
//  Result is a continuous real number; noon on 2000-Jan-01 ≈ JD 2451545.0
// ============================================================
double LunarCalculator::toJulianDay(const QDateTime &utcDt)
{
    int    Y = utcDt.date().year();
    int    M = utcDt.date().month();
    double D = utcDt.date().day()
             + utcDt.time().hour()   / 24.0
             + utcDt.time().minute() / 1440.0
             + utcDt.time().second() / 86400.0;

    if (M <= 2) {
        Y -= 1;
        M += 12;
    }

    int A = Y / 100;
    int B = 2 - A + (A / 4);   // Gregorian correction (positive after 1582)

    return std::floor(365.25  * (Y + 4716))
         + std::floor(30.6001 * (M + 1))
         + D + B - 1524.5;
}

// ============================================================
//  Illuminated fraction from lunar age
//
//  f = (1 − cos θ) / 2,   θ = 2π × age / T
//  f = 0  at new moon, 1 at full moon, 0 again at next new moon.
// ============================================================
double LunarCalculator::illuminationFromAge(double ageDays)
{
    double theta = 2.0 * M_PI * ageDays / SYNODIC;
    return (1.0 - std::cos(theta)) / 2.0;
}

// ============================================================
//  Assign one of eight phases based on age in days
//
//  Each quarter phase is given a ±1-day window centred on its
//  exact age; the in-between periods are the crescents/gibbous.
// ============================================================
LunarPhase LunarCalculator::phaseFromAge(double age)
{
    if (age >= 28.0)                       return LunarPhase::NewMoon;
    if (age <  1.85)                       return LunarPhase::NewMoon;
    if (age <  AGE_FIRST_QUARTER - 0.92)   return LunarPhase::WaxingCrescent;
    if (age <  AGE_FIRST_QUARTER + 0.92)   return LunarPhase::FirstQuarter;
    if (age <  AGE_FULL_MOON     - 0.92)   return LunarPhase::WaxingGibbous;
    if (age <  AGE_FULL_MOON     + 0.92)   return LunarPhase::FullMoon;
    if (age <  AGE_LAST_QUARTER  - 0.92)   return LunarPhase::WaningGibbous;
    if (age <  AGE_LAST_QUARTER  + 0.92)   return LunarPhase::LastQuarter;
    return LunarPhase::WaningCrescent;
}

QString LunarCalculator::phaseToString(LunarPhase p)
{
    switch (p) {
        case LunarPhase::NewMoon:        return "New Moon";
        case LunarPhase::WaxingCrescent: return "Waxing Crescent";
        case LunarPhase::FirstQuarter:   return "First Quarter";
        case LunarPhase::WaxingGibbous:  return "Waxing Gibbous";
        case LunarPhase::FullMoon:       return "Full Moon";
        case LunarPhase::WaningGibbous:  return "Waning Gibbous";
        case LunarPhase::LastQuarter:    return "Last Quarter";
        case LunarPhase::WaningCrescent: return "Waning Crescent";
    }
    return "Unknown";
}

QString LunarCalculator::phaseToEmoji(LunarPhase p)
{
    switch (p) {
        case LunarPhase::NewMoon:        return "\U0001F311"; // 🌑
        case LunarPhase::WaxingCrescent: return "\U0001F312"; // 🌒
        case LunarPhase::FirstQuarter:   return "\U0001F313"; // 🌓
        case LunarPhase::WaxingGibbous:  return "\U0001F314"; // 🌔
        case LunarPhase::FullMoon:       return "\U0001F315"; // 🌕
        case LunarPhase::WaningGibbous:  return "\U0001F316"; // 🌖
        case LunarPhase::LastQuarter:    return "\U0001F317"; // 🌗
        case LunarPhase::WaningCrescent: return "\U0001F318"; // 🌘
    }
    return "\U0001F311";
}

// ============================================================
//  Helper: convert Julian Day → QDateTime (UTC)
//
//  Anchor: JD 2440587.5 = Unix epoch (1970-01-01T00:00:00Z)
// ============================================================
static QDateTime jdToUtcDateTime(double jd)
{
    constexpr double UNIX_EPOCH_JD = 2440587.5;
    double unixSecs = (jd - UNIX_EPOCH_JD) * 86400.0;
    return QDateTime::fromSecsSinceEpoch(static_cast<qint64>(unixSecs), Qt::UTC);
}

// ============================================================
//  Lookup table of the four quarter phases
// ============================================================
struct QuarterInfo {
    double      ageDays;
    const char *name;
};

static const QuarterInfo QUARTERS[] = {
    { 0.0,                        "New Moon"      },
    { 29.53058867 * 0.25,         "First Quarter" },
    { 29.53058867 * 0.50,         "Full Moon"     },
    { 29.53058867 * 0.75,         "Last Quarter"  },
};
static constexpr int N_QUARTERS = 4;

// ============================================================
//  Main entry point
// ============================================================
LunarInfo LunarCalculator::calculate(const QDateTime &utcDt)
{
    LunarInfo info;

    // Step 1 – Julian Day for the requested instant
    double jd = toJulianDay(utcDt);

    // Step 2 – Days elapsed since the reference new moon
    double elapsed = jd - REF_NEW_MOON_JD;

    // Step 3 – Current age within this synodic cycle (0 → SYNODIC)
    double age = std::fmod(elapsed, SYNODIC);
    if (age < 0.0) age += SYNODIC;

    // Step 4 – Fill basic fields
    info.age          = age;
    info.lunarDay     = std::min(static_cast<int>(std::floor(age)) + 1, 29);
    info.illumination = illuminationFromAge(age);
    info.phase        = phaseFromAge(age);
    info.phaseName    = phaseToString(info.phase);
    info.phaseEmoji   = phaseToEmoji(info.phase);

    // ----------------------------------------------------------------
    // Step 5 – Find the most recent quarter phase (previous)
    //
    // For each of the 4 quarters, compute how many days ago it occurred
    // in this or the previous cycle. Take the one with the smallest
    // positive difference.
    // ----------------------------------------------------------------
    int    prevIdx  = 0;
    double prevDiff = SYNODIC + 1.0;   // sentinel

    for (int i = 0; i < N_QUARTERS; ++i) {
        double diff = age - QUARTERS[i].ageDays;
        if (diff < 0.0) diff += SYNODIC;        // went into the previous cycle
        if (diff < prevDiff) {
            prevDiff = diff;
            prevIdx  = i;
        }
    }

    info.prevPhaseName = QString::fromUtf8(QUARTERS[prevIdx].name);
    info.prevPhaseTime = jdToUtcDateTime(jd - prevDiff);

    // ----------------------------------------------------------------
    // Step 6 – Find the next quarter phase
    // ----------------------------------------------------------------
    int    nextIdx  = 0;
    double nextDiff = SYNODIC + 1.0;   // sentinel

    for (int i = 0; i < N_QUARTERS; ++i) {
        double diff = QUARTERS[i].ageDays - age;
        if (diff <= 0.0) diff += SYNODIC;       // already passed; push to next cycle
        if (diff < nextDiff) {
            nextDiff = diff;
            nextIdx  = i;
        }
    }

    info.nextPhaseName = QString::fromUtf8(QUARTERS[nextIdx].name);
    info.nextPhaseTime = jdToUtcDateTime(jd + nextDiff);

    return info;
}
