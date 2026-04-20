#pragma once

#include <QString>
#include <QDateTime>

//8 lunar phases
enum class LunarPhase {
    NewMoon,
    WaxingCrescent,
    FirstQuarter,
    WaxingGibbous,
    FullMoon,
    WaningGibbous,
    LastQuarter,
    WaningCrescent
};

struct LunarInfo {
    double age;            
    int lunarDay;       
    double illumination;   

    LunarPhase phase;
    QString phaseName;
    QString phaseEmoji;     // unicode emoji, change later

    // nearest past quarter phase
    QString prevPhaseName;
    QDateTime prevPhaseTime;  // utc

    // nearest future quarter phase
    QString nextPhaseName;
    QDateTime nextPhaseTime;  // utc
};


class LunarCalculator {
public:
  
    static LunarInfo calculate(const QDateTime &utcDt);

private:
    static double toJulianDay(const QDateTime &utcDt);
    static LunarPhase phaseFromAge(double ageDays);
    static QString phaseToString(LunarPhase p);
    static QString phaseToEmoji(LunarPhase p);
    static double illuminationFromAge(double ageDays);

    // mean synodic month
    static constexpr double SYNODIC = 29.53058867;

    // 2000-Jan-06 18:14 UTC → JD 2451550.2597
    static constexpr double REF_NEW_MOON_JD = 2451550.2597;

    // quarter-phase ages within one synodic month
    static constexpr double AGE_FIRST_QUARTER = SYNODIC * 0.25; // ≈ 7.383
    static constexpr double AGE_FULL_MOON = SYNODIC * 0.50; // ≈ 14.765
    static constexpr double AGE_LAST_QUARTER = SYNODIC * 0.75; // ≈ 22.148
};
