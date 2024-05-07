#ifndef CUSTOMCOLORSTYLE
#define CUSTOMCOLORSTYLE

#include "darkstyle.h"
#include "lightstyle.h"
#include "stylecommon.h"

class CustomColorDarkStyle : public DarkStyle
{
    Q_OBJECT
public:
    explicit CustomColorDarkStyle(const QColor &c) : DarkStyle() {
        color = c.darker();
    }

    void polish(QPalette &palette) {
        int b = color.blue()*96/255;
        int g = color.green()*96/255;
        int r = color.red()*96/255;
        palette.setColor(QPalette::Window, QColor(r*2.65, g*2.65, b*2.65));
        palette.setColor(QPalette::WindowText, QColor(255, 255, 255));
        palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
        palette.setColor(QPalette::Base, QColor(r*2.1, g*2.1, b*2.1));
        palette.setColor(QPalette::AlternateBase, QColor(r*3.65, g*3.65, b*3.65));
        palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
        palette.setColor(QPalette::ToolTipText, QColor(255, 255, 255));
        palette.setColor(QPalette::Text, QColor(255, 255, 255));
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
        palette.setColor(QPalette::Dark, QColor(r*1.75, g*1.75, b*1.75));
        palette.setColor(QPalette::Shadow, QColor(r, g, b));
        palette.setColor(QPalette::Button, QColor(r*2.65, g*2.65, b*2.65));
        palette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
        palette.setColor(QPalette::BrightText, QColor(255, 0, 0));
        palette.setColor(QPalette::Link, QColor(42, 130, 218));
        palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
        palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
        palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));
    }
private:
    QColor color;
};

class CustomColorLightStyle : public LightStyle
{
    Q_OBJECT
public:
    explicit CustomColorLightStyle(const QColor &c) : LightStyle() {
        color = c.darker();
    }

    void polish(QPalette &palette) {
        int b = color.blue()*96/255;
        int g = color.green()*96/255;
        int r = color.red()*96/255;
        palette.setColor(QPalette::Window, QColor(qMin((int)(159+r*1.75),255), qMin((int)(159+g*1.75),255), qMin((int)(159+b*1.75),255)));
        palette.setColor(QPalette::WindowText, QColor(0, 0, 0));
        palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(120, 120, 120));
        palette.setColor(QPalette::Base, QColor(qMin((int)(159+r*2.1),255), qMin((int)(159+g*2.1),255), qMin((int)(159+b*2.1),255)));
        palette.setColor(QPalette::AlternateBase, QColor(qMin((int)(159+r),255), qMin((int)(159+g),255), qMin((int)(159+b),255)));
        palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220));
        palette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
        palette.setColor(QPalette::Text, QColor(0, 0, 0));
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor(120, 120, 120));
        palette.setColor(QPalette::Dark, QColor(qMin((int)(159+r*2.65),255), qMin((int)(159+g*2.65),255), qMin((int)(159+b*2.65),255)));
        palette.setColor(QPalette::Shadow, QColor(qMin((int)(159+r*3.65),255), qMin((int)(159+g*3.65),255), qMin((int)(159+b*3.65),255)));
        palette.setColor(QPalette::Button, QColor(qMin((int)(159+r*1.75),255), qMin((int)(159+g*1.75),255), qMin((int)(159+b*1.75),255)));
        palette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120, 120, 120));
        palette.setColor(QPalette::BrightText, QColor(0, 0, 255));
        palette.setColor(QPalette::Link, QColor(51, 153, 255));
        palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(51, 153, 255));
        palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
        palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(255, 255, 255));
    }

private:
    QColor color;
};

#endif
