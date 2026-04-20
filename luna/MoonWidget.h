#pragma once

#include <QWidget>

class MoonWidget : public QWidget {
    Q_OBJECT
public:
    explicit MoonWidget(QWidget *parent = nullptr);

    void setLunarAge(double ageDays);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double m_age = 0.0;
    static constexpr double SYNODIC = 29.53058867;
};
