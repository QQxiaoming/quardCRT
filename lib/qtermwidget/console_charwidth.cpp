/* $XFree86: xc/programs/xterm/wcwidth.character,v 1.3 2001/07/29 22:08:16 tsi Exp $ */
/*
 * This is an implementation of wcwidth() and wcswidth() as defined in
 * "The Single UNIX Specification, Version 2, The Open Group, 1997"
 * <http://www.UNIX-systems.org/online.html>
 *
 * Markus Kuhn -- 2001-01-12 -- public domain
 */

#include <QString>
#include <QDebug>

#include "utf8proc.h"
#include "console_charwidth.h"

#include <QFont>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QApplication>

//FIXME: Quard: we should refactor this code by using a class 
//       to manage font width info

static QFontMetrics *get_font(void)
{
    static QFontMetrics *fm = nullptr;
    if(!fm) {
        QFont font = QApplication::font();
    #if defined(Q_OS_WIN) && defined(Q_CC_MSVC)
        int fontId = QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/inziu-iosevkaCC-SC-regular.ttf");
    #else
        int fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/font/font/inziu-iosevkaCC-SC-regular.ttf"));
    #endif
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (fontFamilies.size() > 0) {
            font.setFamily(fontFamilies[0]);
        }
        font.setPixelSize(12);
        font.setStyleHint(QFont::Monospace);
        fm = new QFontMetrics(font);
    } 
    return fm;
}

int font_width(wchar_t ucs)
{
    return get_font()->horizontalAdvance(QString(QChar(ucs)),1)/get_font()->horizontalAdvance("0",1);
}

int unicode_width(wchar_t ucs) {
    utf8proc_category_t cat = utf8proc_category( ucs );
    if (cat == UTF8PROC_CATEGORY_CO) {
        // Co: Private use area. libutf8proc makes them zero width, while tmux
        // assumes them to be width 1, and glibc's default width is also 1
        return 1;
    }
    return utf8proc_charwidth( ucs );
}

int wcharwidth(wchar_t ucs)
{
#if 0
    int width = font_width(ucs);
    if(width == 0) {
        width = unicode_width(ucs);
    }
    return width;
#else
    return unicode_width(ucs);
#endif
}

int string_width( const std::wstring & wstr )
{
    int w = 0;
    for ( size_t i = 0; i < wstr.length(); ++i ) {
        w += wcharwidth( wstr[ i ] );
    }
    return w;
}
