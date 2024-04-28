#ifndef CUSTOMCOLORSTYLE
#define CUSTOMCOLORSTYLE

#include "darkstyle.h"
#include "stylecommon.h"

class CustomColorStyle : public DarkStyle
{
    Q_OBJECT
public:
    explicit CustomColorStyle(const QColor &c) : DarkStyle() {
        color = c;
    }

    void polish(QPalette &palette) {
        int b = color.blue();
        int g = color.green();
        int r = color.red();
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

#endif
