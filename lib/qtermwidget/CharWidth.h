#ifndef CHARWIDTH_H
#define CHARWIDTH_H

#include <QString>
#include <QDebug>

#include "utf8proc.h"
#include "CharWidth.h"

#include <QFont>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QApplication>

class CharWidth
{
public:
    CharWidth(QFont font);
    ~CharWidth();

    void setFont(QFont font);
    int font_width(wchar_t ucs);
    int font_width(const QChar & c);
    int string_font_width( const std::wstring & wstr );
    int string_font_width( const QString & str );

    static int unicode_width(wchar_t ucs);
    static int unicode_width(const QChar & c);
    static int string_unicode_width( const std::wstring & wstr );
    static int string_unicode_width( const QString & str );

private:
    QFontMetrics *fm;
};

#endif // CHARWIDTH_H
