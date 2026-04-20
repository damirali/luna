#include <QApplication>
#include <QPalette>
#include <QColor>
#include "MainWindow.h"


static void applyDarkPalette(QApplication &app) 
{
    app.setStyle("Fusion");

    QPalette p;

    // background
    p.setColor(QPalette::Window,        QColor( 38,  40,  52));
    p.setColor(QPalette::WindowText,    QColor(220, 220, 220));
    p.setColor(QPalette::Base,          QColor( 48,  50,  64));
    p.setColor(QPalette::AlternateBase, QColor( 38,  40,  52));

    // text
    p.setColor(QPalette::Text,          QColor(220, 220, 220));
    p.setColor(QPalette::BrightText,    Qt::white);
    p.setColor(QPalette::PlaceholderText, QColor(130, 130, 150));

    // buttons
    p.setColor(QPalette::Button,        QColor( 58,  62,  80));
    p.setColor(QPalette::ButtonText,    QColor(220, 220, 220));

    // selection
    p.setColor(QPalette::Highlight,     QColor( 85,  95, 170));
    p.setColor(QPalette::HighlightedText, Qt::white);

    // borders 
    p.setColor(QPalette::Mid,           QColor( 55,  58,  75));
    p.setColor(QPalette::Dark,          QColor( 28,  30,  42));
    p.setColor(QPalette::Shadow,        QColor( 15,  16,  24));
    p.setColor(QPalette::Light,         QColor( 70,  74,  95));

    // tooltips
    p.setColor(QPalette::ToolTipBase,   QColor( 50,  54,  70));
    p.setColor(QPalette::ToolTipText,   QColor(220, 220, 220));

    app.setPalette(p);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Lunar Calendar");
    app.setApplicationVersion("1.0");

    applyDarkPalette(app); //may be commented out

    MainWindow w;
    w.show();

    return app.exec();
}
