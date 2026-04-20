#include "MainWindow.h"
#include "MoonWidget.h"
#include "LunarCalculator.h"

#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTimer>
#include <QTimeZone>
#include <QDateTime>
#include <QFrame>
#include <QFont>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QList>
#include <cmath>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_tzId("UTC")
{
    setWindowTitle("Lunar Calendar");
    setMinimumSize(460, 620);
    resize(500, 660);

    buildUi();
    populateTimezones();

    // 1-second timer keeps the clock and lunar data live
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onRefresh);
    m_timer->start();

    // draw immediately on startup
    onRefresh();
}


void MainWindow::buildUi()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *root = new QVBoxLayout(central);
    root->setSpacing(10);
    root->setContentsMargins(14, 14, 14, 14);


    QLabel *title = new QLabel("Lunar Calendar", this);
    {
        QFont f = title->font();
        f.setPointSize(20);
        f.setBold(true);
        title->setFont(f);
        title->setAlignment(Qt::AlignCenter);
    }
    root->addWidget(title);

    QGroupBox *inputBox = new QGroupBox("Location & Timezone", this);
    QGridLayout *ig = new QGridLayout(inputBox);
    ig->setColumnStretch(1, 1);

    //change ts
    ig->addWidget(new QLabel("Location:", this), 0, 0);
    m_locationEdit = new QLineEdit("My City", this);
    m_locationEdit->setPlaceholderText("City or location name (display only)");
    ig->addWidget(m_locationEdit, 0, 1, 1, 2);

    ig->addWidget(new QLabel("Timezone:", this), 1, 0);
    m_tzCombo = new QComboBox(this);
    ig->addWidget(m_tzCombo, 1, 1);

    //change this too
    m_updateBtn = new QPushButton("Update", this);
    m_updateBtn->setFixedWidth(75);
    ig->addWidget(m_updateBtn, 1, 2);

    root->addWidget(inputBox);

    
    m_moonWidget = new MoonWidget(this);
    m_moonWidget->setFixedSize(200, 200);

    QHBoxLayout *moonRow = new QHBoxLayout();
    moonRow->addStretch();
    moonRow->addWidget(m_moonWidget);
    moonRow->addStretch();
    root->addLayout(moonRow);

    
    QGroupBox *infoBox = new QGroupBox("Lunar Information", this);
    QGridLayout *info = new QGridLayout(infoBox);
    info->setVerticalSpacing(7);
    info->setColumnStretch(1, 1);

    // helper lambdas
    auto makeKey = [this](const QString &text) -> QLabel * {
        auto *l = new QLabel(text, this);
        QFont f = l->font();
        f.setBold(true);
        l->setFont(f);
        return l;
    };
    auto makeVal = [this]() -> QLabel * {
        auto *l = new QLabel("—", this);
        return l;
    };

    int row = 0;

    info->addWidget(makeKey("Local Time:"),     row, 0);
    m_timeLabel = makeVal();
    info->addWidget(m_timeLabel, row++, 1);

    info->addWidget(makeKey("Lunar Day:"),      row, 0);
    m_lunarDayLabel = makeVal();
    info->addWidget(m_lunarDayLabel, row++, 1);

    info->addWidget(makeKey("Illumination:"),   row, 0);
    m_illuminationLabel = makeVal();
    info->addWidget(m_illuminationLabel, row++, 1);

    // horizontal line
    QFrame *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    info->addWidget(sep, row++, 0, 1, 2);

    info->addWidget(makeKey("Current Phase:"),  row, 0);
    m_currentPhaseLabel = makeVal();
    info->addWidget(m_currentPhaseLabel, row++, 1);

    info->addWidget(makeKey("Previous Phase:"), row, 0);
    m_prevPhaseLabel = makeVal();
    info->addWidget(m_prevPhaseLabel, row++, 1);

    info->addWidget(makeKey("Next Phase:"),     row, 0);
    m_nextPhaseLabel = makeVal();
    info->addWidget(m_nextPhaseLabel, row++, 1);

    root->addWidget(infoBox);



    // signal connections
    connect(m_updateBtn,    &QPushButton::clicked,
            this,           &MainWindow::onRefresh);
    connect(m_locationEdit, &QLineEdit::returnPressed,
            this,           &MainWindow::onRefresh);
    connect(m_tzCombo,      QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,           &MainWindow::onRefresh);
}

//add and change
void MainWindow::populateTimezones()
{
    static const QList<QByteArray> ZONES = {
        "Pacific/Honolulu",    // UTC-10
        "America/Anchorage",   // UTC-9
        "America/Los_Angeles", // UTC-8/-7
        "America/Denver",      // UTC-7/-6
        "America/Chicago",     // UTC-6/-5
        "America/New_York",    // UTC-5/-4
        "America/Caracas",     // UTC-4
        "America/Sao_Paulo",   // UTC-3
        "Atlantic/Azores",     // UTC-1/0
        "UTC",                 // UTC+0
        "Europe/London",       // UTC+0/+1
        "Europe/Paris",        // UTC+1/+2
        "Europe/Athens",       // UTC+2/+3
        "Europe/Moscow",       // UTC+3
        "Asia/Dubai",          // UTC+4
        "Asia/Aqtobe",         // UTC+5 
        "Asia/Karachi",        // UTC+5
        "Asia/Kolkata",        // UTC+5:30
        "Asia/Almaty",         // UTC+6
        "Asia/Dhaka",          // UTC+6
        "Asia/Bangkok",        // UTC+7
        "Asia/Shanghai",       // UTC+8
        "Asia/Seoul",          // UTC+9
        "Asia/Tokyo",          // UTC+9
        "Australia/Sydney",    // UTC+10/+11
        "Pacific/Auckland",    // UTC+12/+13
    };

    QDateTime now = QDateTime::currentDateTimeUtc();

    m_tzCombo->blockSignals(true);
    m_tzCombo->clear();

    for (const QByteArray &tzId : ZONES) {
        QTimeZone tz(tzId);
        if (!tz.isValid()) continue;

        int offsetSecs = tz.offsetFromUtc(now);
        int absSeconds = std::abs(offsetSecs);
        int absHours = absSeconds / 3600;
        int absMins = (absSeconds % 3600) / 60;
        char sign = (offsetSecs >= 0) ? '+' : '-';

        QString label = QString("UTC%1%2:%3  %4")
            .arg(sign)
            .arg(absHours, 2, 10, QChar('0'))
            .arg(absMins,  2, 10, QChar('0'))
            .arg(QString::fromUtf8(tzId));

        m_tzCombo->addItem(label, QString::fromUtf8(tzId));
    }

    m_tzCombo->blockSignals(false);

    int utcIdx = m_tzCombo->findData(QString("UTC"));
    if (utcIdx >= 0) m_tzCombo->setCurrentIndex(utcIdx);
}


void MainWindow::onRefresh()
{
    // read the currently selected timezone
    QString tzStr = m_tzCombo->currentData().toString();
    if (!tzStr.isEmpty()) m_tzId = tzStr.toUtf8();

    QTimeZone tz(m_tzId);
    if (!tz.isValid()) tz = QTimeZone::utc();

    QDateTime utcNow   = QDateTime::currentDateTimeUtc();
    QDateTime localNow = utcNow.toTimeZone(tz);

    LunarInfo info = LunarCalculator::calculate(utcNow);

    m_moonWidget->setLunarAge(info.age);
    updateDisplay(info, localNow);
}


void MainWindow::updateDisplay(const LunarInfo &info, const QDateTime &localDt)
{
    QTimeZone tz(m_tzId);
    QString   tzAbbr = tz.isValid() ? tz.abbreviation(localDt) : "UTC";

    // local time
    m_timeLabel->setText(
        localDt.toString("yyyy-MM-dd   hh:mm:ss") + "  (" + tzAbbr + ")");

    // lunar day and age
    m_lunarDayLabel->setText(
        QString("Day %1 of 29  |  age: %2 days")
            .arg(info.lunarDay)
            .arg(info.age, 0, 'f', 2));

    // illumination
    int pct = static_cast<int>(std::round(info.illumination * 100.0));
    m_illuminationLabel->setText(QString("%1 %").arg(pct));

    // current phase
    m_currentPhaseLabel->setText(info.phaseEmoji + "  " + info.phaseName);

    // previous phase 
    {
        QDateTime prevLocal = info.prevPhaseTime.toTimeZone(tz);

        // change ts
        qint64 secsDiff = QDateTime::currentSecsSinceEpoch()
                        - info.prevPhaseTime.toSecsSinceEpoch();
        int daysAgo = static_cast<int>(secsDiff / 86400);

        QString agoStr = (daysAgo == 0) ? "today"
                       : (daysAgo == 1) ? "yesterday"
                       : QString("%1 days ago").arg(daysAgo);

        m_prevPhaseLabel->setText(
            info.prevPhaseName + "  ("
            + prevLocal.toString("MMM d, yyyy") + ",  " + agoStr + ")");
    }

    // next phase 
    {
        QDateTime nextLocal = info.nextPhaseTime.toTimeZone(tz);

        qint64 secsUntil = info.nextPhaseTime.toSecsSinceEpoch()
                         - QDateTime::currentSecsSinceEpoch();

        QString untilStr;
        if (secsUntil <= 0) {
            untilStr = "very soon";
        } else if (secsUntil < 3600) {
            untilStr = QString("in %1 min").arg(secsUntil / 60);
        } else if (secsUntil < 86400) {
            untilStr = QString("in ~%1 h").arg(secsUntil / 3600);
        } else {
            untilStr = QString("in ~%1 days").arg(secsUntil / 86400);
        }

        m_nextPhaseLabel->setText(
            info.nextPhaseName + "  ("
            + nextLocal.toString("MMM d, yyyy") + ",  " + untilStr + ")");
    }
}
