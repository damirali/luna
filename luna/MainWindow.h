#pragma once

#include <QMainWindow>
#include <QByteArray>
#include "LunarCalculator.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;
class QTimer;
QT_END_NAMESPACE

class MoonWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    // called by the Update button 
    void onRefresh();

private:
    void buildUi();
    void populateTimezones();
    void updateDisplay(const LunarInfo &info, const QDateTime &localDt);

    // input widgets
    QLineEdit   *m_locationEdit;
    QComboBox   *m_tzCombo;
    QPushButton *m_updateBtn;

    // display labels
    QLabel *m_timeLabel;
    QLabel *m_lunarDayLabel;
    QLabel *m_illuminationLabel;
    QLabel *m_currentPhaseLabel;
    QLabel *m_prevPhaseLabel;
    QLabel *m_nextPhaseLabel;

    // moon rendering
    MoonWidget *m_moonWidget;

    // refresh
    QTimer *m_timer;

    // current zone
    QByteArray m_tzId;
};
