#include "CharWidth.h"

CharWidth::CharWidth(QFont font)
{
    fm = new QFontMetrics(font);
}

CharWidth::~CharWidth()
{
    delete fm;
}

void CharWidth::setFont(QFont font)
{
    delete fm;
    fm = new QFontMetrics(font);
}

int CharWidth::font_width(wchar_t ucs)
{
    return fm->horizontalAdvance(QString(QChar(ucs)),1)/fm->horizontalAdvance("0",1);
}

int CharWidth::font_width(const QChar & c)
{
    return fm->horizontalAdvance(c,1)/fm->horizontalAdvance("0",1);
}

int CharWidth::string_font_width( const std::wstring & wstr )
{
    int width = 0;
    for (auto & c : wstr) {
        width += font_width(c);
    }
    return width;
}

int CharWidth::string_font_width( const QString & str )
{
    int width = 0;
    for (auto & c : str) {
        width += font_width(c.unicode());
    }
    return width;
}

int CharWidth::unicode_width(wchar_t ucs)
{
    utf8proc_category_t cat = utf8proc_category( ucs );
    if (cat == UTF8PROC_CATEGORY_CO) {
        // Co: Private use area. libutf8proc makes them zero width, while tmux
        // assumes them to be width 1, and glibc's default width is also 1
        return 1;
    }
    return utf8proc_charwidth( ucs );
}

int CharWidth::unicode_width(const QChar & c)
{
    return unicode_width(c.unicode());
}

int CharWidth::string_unicode_width( const std::wstring & wstr )
{
    int width = 0;
    for (auto & c : wstr) {
        width += unicode_width(c);
    }
    return width;
}

int CharWidth::string_unicode_width( const QString & str )
{
    int width = 0;
    for (auto & c : str) {
        width += unicode_width(c.unicode());
    }
    return width;
}

