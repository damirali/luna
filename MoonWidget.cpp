#include "MoonWidget.h"

#include <QPainter>
#include <QImage>
#include <QColor>
#include <algorithm>
#include <cmath>

MoonWidget::MoonWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(160, 160);
    // transparent background so the parent window shows through
    setAttribute(Qt::WA_OpaquePaintEvent, false);
}

QSize MoonWidget::sizeHint() const
{
    return QSize(200, 200);
}

void MoonWidget::setLunarAge(double ageDays)
{
    m_age = ageDays;
    update();   // schedule a repaint
}


void MoonWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);


    int side = std::min(width(), height()) - 12;
    if (side < 20) return;

    int    originX = (width()  - side) / 2;
    int    originY = (height() - side) / 2;
    double R       = side / 2.0;

    // phase angle
    double theta = 2.0 * M_PI * m_age / SYNODIC;

    QImage moonImg(side, side, QImage::Format_ARGB32_Premultiplied);
    moonImg.fill(Qt::transparent);

    // lit side: warm white (255, 248, 200)
    // dark side: deep midnight (20, 24, 50)
    const QRgb litRgb  = qPremultiply(qRgba(255, 248, 200, 255));
    const QRgb darkRgb = qPremultiply(qRgba( 20,  24,  50, 255));

    double cosTheta = std::cos(theta);

    for (int py = 0; py < side; ++py) {
        double y    = py - R + 0.5;            // sub-pixel centre, relative to moon centre
        double xe_sq = R * R - y * y;
        if (xe_sq < 0.0) continue;            // outside the bounding box rows
        double xe = std::sqrt(xe_sq);          // half-chord width at this scan line

        QRgb *row = reinterpret_cast<QRgb *>(moonImg.scanLine(py));

        for (int px = 0; px < side; ++px) {
            double x     = px - R + 0.5;       // relative to moon centre
            double dist2 = x * x + y * y;

            if (dist2 > R * R) continue;       // outside the lunar disk

            bool lit;
            if (theta <= M_PI) {
                lit = (x >= cosTheta * xe);
            } else {
                lit = (x <= -cosTheta * xe);
            }

            row[px] = lit ? litRgb : darkRgb;
        }
    }

    // background
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(12, 14, 35));
    painter.drawEllipse(originX - 3, originY - 3, side + 6, side + 6);

    // moon image
    painter.drawImage(originX, originY, moonImg);

    // outer rim
    painter.setPen(QPen(QColor(70, 75, 110), 1.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(originX, originY, side, side);
}
