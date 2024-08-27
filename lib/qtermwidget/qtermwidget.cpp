/*  Copyright (C) 2008 e_k (e_k@users.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include <QLayout>
#include <QBoxLayout>
#include <QtDebug>
#include <QDir>
#include <QMessageBox>
#include <QRegularExpression>

#include "ColorTables.h"
#include "Session.h"
#include "Screen.h"
#include "ScreenWindow.h"
#include "Emulation.h"
#include "TerminalDisplay.h"
#include "KeyboardTranslator.h"
#include "ColorScheme.h"
#include "SearchBar.h"
#include "qtermwidget.h"

#ifdef Q_OS_MACOS
// Qt does not support fontconfig on macOS, so we need to use a "real" font name.
#define DEFAULT_FONT_FAMILY                   "Menlo"
#else
#define DEFAULT_FONT_FAMILY                   "Monospace"
#endif

#define STEP_ZOOM 3

QTermWidget::QTermWidget(QWidget *messageParentWidget, QWidget *parent)
    : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);

    m_session = new Session(this);
    m_session->setCodec(QStringEncoder{QStringConverter::Encoding::Utf8});
    m_session->setFlowControlEnabled(true);
    m_session->setHistoryType(HistoryTypeBuffer(1000));
    m_session->setDarkBackground(true);
    m_session->setKeyBindings(QString());

    m_terminalDisplay = new TerminalDisplay(this);
    m_terminalDisplay->setBellMode(TerminalDisplay::NotifyBell);
    m_terminalDisplay->setTerminalSizeHint(true);
    m_terminalDisplay->setTripleClickMode(TerminalDisplay::SelectWholeLine);
    m_terminalDisplay->setTerminalSizeStartup(true);
    m_terminalDisplay->setRandomSeed(m_session->sessionId() * 31);

    m_layout->addWidget(m_terminalDisplay);
    m_terminalDisplay->setObjectName("terminalDisplay");
    setMessageParentWidget(messageParentWidget?messageParentWidget:this);

    connect(m_session, &Session::bellRequest, m_terminalDisplay, &TerminalDisplay::bell);
    connect(m_terminalDisplay, &TerminalDisplay::notifyBell, this, &QTermWidget::bell);
    connect(m_terminalDisplay, &TerminalDisplay::handleCtrlC, this, &QTermWidget::handleCtrlC);
    connect(m_terminalDisplay, &TerminalDisplay::changedContentCountSignal, this, &QTermWidget::termSizeChange);
    connect(m_terminalDisplay, &TerminalDisplay::mousePressEventForwarded, this, &QTermWidget::mousePressEventForwarded);
    connect(m_session, &Session::activity, this, &QTermWidget::activity);
    connect(m_session, &Session::silence, this, &QTermWidget::silence);
    connect(m_session, &Session::profileChangeCommandReceived, this, &QTermWidget::profileChanged);
    connect(m_session, &Session::receivedData, this, &QTermWidget::receivedData);
    connect(m_session->emulation(), &Emulation::zmodemRecvDetected, this, &QTermWidget::zmodemRecvDetected);
    connect(m_session->emulation(), &Emulation::zmodemSendDetected, this, &QTermWidget::zmodemSendDetected);
             
    // That's OK, FilterChain's dtor takes care of UrlFilter.
    urlFilter = new UrlFilter();
    connect(urlFilter, &UrlFilter::activated, this, &QTermWidget::urlActivated);
    m_terminalDisplay->filterChain()->addFilter(urlFilter);
    m_UrlFilterEnable = true;

    m_searchBar = new SearchBar(this);
    m_searchBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    connect(m_searchBar, &SearchBar::searchCriteriaChanged, this, [this](){
        search(true, false);
    });
    connect(m_searchBar, &SearchBar::findNext, this, [this](){
        search(true, true);
    });
    connect(m_searchBar, &SearchBar::findPrevious, this, [this](){
        search(false, false);
    });
    m_layout->addWidget(m_searchBar);
    m_searchBar->hide();
    QString style_sheet = qApp->styleSheet();
    m_searchBar->setStyleSheet(style_sheet);
    
    this->setFocus( Qt::OtherFocusReason );
    this->setFocusPolicy( Qt::WheelFocus );
    m_terminalDisplay->resize(this->size());

    this->setFocusProxy(m_terminalDisplay);
    connect(m_terminalDisplay, &TerminalDisplay::copyAvailable,
            this, &QTermWidget::copyAvailable);
    connect(m_terminalDisplay, &TerminalDisplay::termGetFocus,
            this, &QTermWidget::termGetFocus);
    connect(m_terminalDisplay, &TerminalDisplay::termLostFocus,
            this, &QTermWidget::termLostFocus);
    connect(m_terminalDisplay, &TerminalDisplay::keyPressedSignal, this,
            [this] (QKeyEvent* e, bool) { emit termKeyPressed(e); });

    QFont font = QApplication::font();
    font.setFamily(QLatin1String(DEFAULT_FONT_FAMILY));
    font.setPointSize(10);
    font.setStyleHint(QFont::TypeWriter);
    setTerminalFont(font);
    m_searchBar->setFont(font);

    setScrollBarPosition(NoScrollBar);
    setKeyboardCursorShape(Emulation::KeyboardCursorShape::BlockCursor);

    m_session->addView(m_terminalDisplay);

    connect(m_session, &Session::resizeRequest, this, [this](const QSize & size){
        setSize(size);
    });
    connect(m_session, &Session::finished, this, &QTermWidget::finished);
    connect(m_session, &Session::titleChanged, this, &QTermWidget::titleChanged);
    connect(m_session, &Session::cursorChanged, this, &QTermWidget::cursorChanged);
}

QTermWidget::~QTermWidget()
{
    setUrlFilterEnabled(true);
    clearHighLightTexts();
    delete m_searchBar;
    emit destroyed();
}

void QTermWidget::selectionChanged(bool textSelected)
{
    emit copyAvailable(textSelected);
}

void QTermWidget::search(bool forwards, bool next)
{
    int startColumn, startLine;

    if (next) // search from just after current selection
    {
        m_terminalDisplay->screenWindow()->screen()->getSelectionEnd(startColumn, startLine);
        startColumn++;
    }
    else // search from start of current selection
    {
        m_terminalDisplay->screenWindow()->screen()->getSelectionStart(startColumn, startLine);
    }

    //qDebug() << "current selection starts at: " << startColumn << startLine;
    //qDebug() << "current cursor position: " << m_terminalDisplay->screenWindow()->cursorPosition();

    QRegularExpression regExp;
    if (m_searchBar->useRegularExpression()) {
        regExp.setPattern(m_searchBar->searchText());
    } else {
        regExp.setPattern(QRegularExpression::escape(m_searchBar->searchText()));
    }
    regExp.setPatternOptions(m_searchBar->matchCase() ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);

    HistorySearch *historySearch =
            new HistorySearch(m_session->emulation(), regExp, forwards, startColumn, startLine, this);
    connect(historySearch, &HistorySearch::matchFound, this, [this](int startColumn, int startLine, int endColumn, int endLine){
        ScreenWindow* sw = m_terminalDisplay->screenWindow();
        //qDebug() << "Scroll to" << startLine;
        sw->scrollTo(startLine);
        sw->setTrackOutput(false);
        sw->notifyOutputChanged();
        sw->setSelectionStart(startColumn, startLine - sw->currentLine(), false);
        sw->setSelectionEnd(endColumn, endLine - sw->currentLine());
    });
    connect(historySearch, &HistorySearch::noMatchFound, this, [this](){
        m_terminalDisplay->screenWindow()->clearSelection();
    });
    connect(historySearch, &HistorySearch::noMatchFound, m_searchBar, &SearchBar::noMatchFound);
    historySearch->search();
}

QSize QTermWidget::sizeHint() const
{
    QSize size = m_terminalDisplay->sizeHint();
    size.rheight() = 150;
    return size;
}

void QTermWidget::setTerminalSizeHint(bool enabled)
{
    m_terminalDisplay->setTerminalSizeHint(enabled);
}

bool QTermWidget::terminalSizeHint()
{
    return m_terminalDisplay->terminalSizeHint();
}

void QTermWidget::startTerminalTeletype()
{
    m_session->runEmptyPTY();
    // redirect data from TTY to external recipient
    connect( m_session->emulation(), &Emulation::sendData, this, [this](const char *buff, int len) {
        if (m_echo) {
            recvData(buff, len);
        }
        emit sendData(buff, len);
    });
    connect( m_session->emulation(), &Emulation::dupDisplayOutput, this, &QTermWidget::dupDisplayOutput);
}

void QTermWidget::setTerminalFont(const QFont &font)
{
    m_terminalDisplay->setVTFont(font);
}

QFont QTermWidget::getTerminalFont()
{
    return m_terminalDisplay->getVTFont();
}

void QTermWidget::setTerminalOpacity(qreal level)
{
    m_terminalDisplay->setOpacity(level);
}

void QTermWidget::setTerminalBackgroundImage(const QString& backgroundImage)
{
    m_terminalDisplay->setBackgroundImage(backgroundImage);
}

void QTermWidget::setTerminalBackgroundMovie(const QString& backgroundMovie)
{
    m_terminalDisplay->setBackgroundMovie(backgroundMovie);
}

void QTermWidget::setTerminalBackgroundVideo(const QString& backgroundVideo)
{
    m_terminalDisplay->setBackgroundVideo(backgroundVideo);
}

void QTermWidget::setTerminalBackgroundMode(int mode)
{
    m_terminalDisplay->setBackgroundMode((BackgroundMode)mode);
}

void QTermWidget::setTextCodec(QStringEncoder codec)
{
    if (!m_session)
        return;
    m_session->setCodec(std::move(codec));
}

void QTermWidget::setColorScheme(const QString& origName)
{
    const ColorScheme *cs = nullptr;

    const bool isFile = QFile::exists(origName);
    const QString& name = isFile ?
            QFileInfo(origName).baseName() :
            origName;

    // avoid legacy (int) solution
    if (!availableColorSchemes().contains(name))
    {
        if (isFile)
        {
            if (ColorSchemeManager::instance()->loadCustomColorScheme(origName))
                cs = ColorSchemeManager::instance()->findColorScheme(name);
            else
                qWarning () << Q_FUNC_INFO
                        << "cannot load color scheme from"
                        << origName;
        }

        if (!cs)
            cs = ColorSchemeManager::instance()->defaultColorScheme();
    }
    else
        cs = ColorSchemeManager::instance()->findColorScheme(name);

    if (! cs)
    {
        QMessageBox::information(messageParentWidget,
                                 tr("Color Scheme Error"),
                                 tr("Cannot load color scheme: %1").arg(name));
        return;
    }
    ColorEntry table[TABLE_COLORS];
    cs->getColorTable(table);
    m_terminalDisplay->setColorTable(table);
    m_session->setDarkBackground(cs->hasDarkBackground());
}

QStringList QTermWidget::getAvailableColorSchemes()
{
   return QTermWidget::availableColorSchemes();
}

QStringList QTermWidget::availableColorSchemes()
{
    QStringList ret;
    const auto allColorSchemes = ColorSchemeManager::instance()->allColorSchemes();
    for (const ColorScheme* cs : allColorSchemes)
        ret.append(cs->name());
    return ret;
}

void QTermWidget::addCustomColorSchemeDir(const QString& custom_dir)
{
    ColorSchemeManager::instance()->addCustomColorSchemeDir(custom_dir);
}

void QTermWidget::setBackgroundColor(const QColor &color)
{
    m_terminalDisplay->setBackgroundColor(color);
}

void QTermWidget::setForegroundColor(const QColor &color)
{
    m_terminalDisplay->setForegroundColor(color);
}

void QTermWidget::setANSIColor(const int ansiColorId, const QColor &color)
{
    m_terminalDisplay->setColorTableColor(ansiColorId, color);
}

void QTermWidget::setPreeditColorIndex(int index)
{
    m_terminalDisplay->setPreeditColorIndex(index);
}

void QTermWidget::setSize(const QSize &size)
{
    m_terminalDisplay->setSize(size.width(), size.height());
}

void QTermWidget::setHistorySize(int lines)
{
    if (lines < 0)
        m_session->setHistoryType(HistoryTypeFile());
    else if (lines == 0)
        m_session->setHistoryType(HistoryTypeNone());
    else
        m_session->setHistoryType(HistoryTypeBuffer(lines));
}

int QTermWidget::historySize() const
{
    const HistoryType& currentHistory = m_session->historyType();

     if (currentHistory.isEnabled()) {
         if (currentHistory.isUnlimited()) {
             return -1;
         } else {
             return currentHistory.maximumLineCount();
         }
     } else {
         return 0;
     }
}

void QTermWidget::setScrollBarPosition(ScrollBarPosition pos)
{
    m_terminalDisplay->setScrollBarPosition(pos);
}

void QTermWidget::scrollToEnd()
{
    m_terminalDisplay->scrollToEnd();
}

void QTermWidget::sendText(const QString &text)
{
    m_session->sendText(text);
}

void QTermWidget::sendKeyEvent(QKeyEvent *e)
{
    m_session->sendKeyEvent(e);
}

void QTermWidget::resizeEvent(QResizeEvent*)
{
    //qDebug("global window resizing...with %d %d", this->size().width(), this->size().height());
    m_terminalDisplay->resize(this->size());
}

void QTermWidget::sessionFinished()
{
    emit finished();
}

void QTermWidget::bracketText(QString& text)
{
    m_terminalDisplay->bracketText(text);
}

void QTermWidget::disableBracketedPasteMode(bool disable)
{
    m_terminalDisplay->disableBracketedPasteMode(disable);
}

bool QTermWidget::bracketedPasteModeIsDisabled() const
{
    return m_terminalDisplay->bracketedPasteModeIsDisabled();
}

void QTermWidget::copyClipboard()
{
    m_terminalDisplay->copyClipboard(QClipboard::Clipboard);
}

void QTermWidget::copySelection()
{
    m_terminalDisplay->copyClipboard(QClipboard::Selection);
}

void QTermWidget::pasteClipboard()
{
    m_terminalDisplay->pasteClipboard();
}

void QTermWidget::pasteSelection()
{
    m_terminalDisplay->pasteSelection();
}

void QTermWidget::selectAll()
{
    m_terminalDisplay->selectAll();
}

int QTermWidget::setZoom(int step)
{
    QFont font = m_terminalDisplay->getVTFont();

    font.setPointSize(font.pointSize() + step);
    setTerminalFont(font);
    return font.pointSize();
}

int QTermWidget::zoomIn()
{
    return setZoom(STEP_ZOOM);
}

int QTermWidget::zoomOut()
{
    return setZoom(-STEP_ZOOM);
}

void QTermWidget::setKeyBindings(const QString & kb)
{
    m_session->setKeyBindings(kb);
}

void QTermWidget::clear()
{
    clearScreen();
    clearScrollback();
}

void QTermWidget::clearScrollback()
{
    m_session->clearHistory();
}

void QTermWidget::clearScreen()
{
    m_session->emulation()->reset();
    m_session->refresh();
}

void QTermWidget::setFlowControlEnabled(bool enabled)
{
    m_session->setFlowControlEnabled(enabled);
}

bool QTermWidget::flowControlEnabled(void)
{
    return m_session->flowControlEnabled();
}

void QTermWidget::setFlowControlWarningEnabled(bool enabled)
{
    if (flowControlEnabled()) {
        // Do not show warning label if flow control is disabled
        m_terminalDisplay->setFlowControlWarningEnabled(enabled);
    }
}

QStringList QTermWidget::availableKeyBindings()
{
    return KeyboardTranslatorManager::instance()->allTranslators();
}

QString QTermWidget::keyBindings()
{
    return m_session->keyBindings();
}

void QTermWidget::toggleShowSearchBar()
{
    if(m_searchBar->isHidden()) {
        m_searchBar->setText(selectedText(true));
        m_searchBar->show();
    } else {
        m_searchBar->hide();
    }
}

void QTermWidget::setMotionAfterPasting(int action)
{
    m_terminalDisplay->setMotionAfterPasting((MotionAfterPasting) action);
}

int QTermWidget::historyLinesCount()
{
    return m_terminalDisplay->screenWindow()->screen()->getHistLines();
}

int QTermWidget::screenColumnsCount()
{
    return m_terminalDisplay->screenWindow()->screen()->getColumns();
}

int QTermWidget::screenLinesCount()
{
    return m_terminalDisplay->screenWindow()->screen()->getLines();
}

void QTermWidget::setSelectionStart(int row, int column)
{
    m_terminalDisplay->screenWindow()->screen()->setSelectionStart(column, row, true);
}

void QTermWidget::setSelectionEnd(int row, int column)
{
    m_terminalDisplay->screenWindow()->screen()->setSelectionEnd(column, row);
}

void QTermWidget::getSelectionStart(int& row, int& column)
{
    m_terminalDisplay->screenWindow()->screen()->getSelectionStart(column, row);
}

void QTermWidget::getSelectionEnd(int& row, int& column)
{
    m_terminalDisplay->screenWindow()->screen()->getSelectionEnd(column, row);
}

QString QTermWidget::selectedText(bool preserveLineBreaks)
{
    return m_terminalDisplay->screenWindow()->screen()->selectedText(preserveLineBreaks);
}

void QTermWidget::setMonitorActivity(bool enabled)
{
    m_session->setMonitorActivity(enabled);
}

void QTermWidget::setMonitorSilence(bool enabled)
{
    m_session->setMonitorSilence(enabled);
}

void QTermWidget::setSilenceTimeout(int seconds)
{
    m_session->setMonitorSilenceSeconds(seconds);
}

Filter::HotSpot* QTermWidget::getHotSpotAt(const QPoint &pos) const
{
    int row = 0, column = 0;
    m_terminalDisplay->getCharacterPosition(pos, row, column);
    return getHotSpotAt(row, column);
}

Filter::HotSpot* QTermWidget::getHotSpotAt(int row, int column) const
{
    return m_terminalDisplay->filterChain()->hotSpotAt(row, column);
}

QList<QAction*> QTermWidget::filterActions(const QPoint& position)
{
    return m_terminalDisplay->filterActions(position);
}

int QTermWidget::recvData(const char *buff, int len) const
{
    return m_session->recvData(buff,len);
}

void QTermWidget::setKeyboardCursorShape(KeyboardCursorShape shape)
{
    m_terminalDisplay->setKeyboardCursorShape(shape);
}

void QTermWidget::setKeyboardCursorShape(uint32_t shape)
{
    m_terminalDisplay->setKeyboardCursorShape((KeyboardCursorShape)shape);
}

void QTermWidget::setBlinkingCursor(bool blink)
{
    m_terminalDisplay->setBlinkingCursor(blink);
}

void QTermWidget::setBidiEnabled(bool enabled)
{
    m_terminalDisplay->setBidiEnabled(enabled);
}

bool QTermWidget::isBidiEnabled()
{
    return m_terminalDisplay->isBidiEnabled();
}

void QTermWidget::cursorChanged(Emulation::KeyboardCursorShape cursorShape, bool blinkingCursorEnabled)
{
    // TODO: A switch to enable/disable DECSCUSR?
    setKeyboardCursorShape(cursorShape);
    setBlinkingCursor(blinkingCursorEnabled);
}

void QTermWidget::setMargin(int margin)
{
    m_terminalDisplay->setMargin(margin);
}

int QTermWidget::getMargin() const
{
    return m_terminalDisplay->margin();
}

void QTermWidget::saveHistory(QTextStream *stream, int format, int start, int end)
{
    TerminalCharacterDecoder *decoder;
    if(format == 0) {
        decoder = new PlainTextDecoder;
    } else {
        decoder = new HTMLDecoder;
    }
    decoder->begin(stream);
    if(start < 0) {
        start = 0;
    }
    if(end < 0) {
        end = m_session->emulation()->lineCount();
    }
    m_session->emulation()->writeToStream(decoder, start, end);
    delete decoder;
}

void QTermWidget::saveHistory(QIODevice *device, int format, int start, int end)
{
    QTextStream stream(device);
    saveHistory(&stream, format, start, end);
}

void QTermWidget::screenShot(QPixmap *pixmap)
{
    QPixmap currPixmap(m_terminalDisplay->size());
    m_terminalDisplay->render(&currPixmap);
    *pixmap = currPixmap.scaled(pixmap->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void QTermWidget::repaintDisplay(void)
{
    m_terminalDisplay->repaintDisplay();
}

void QTermWidget::screenShot(const QString &fileName)
{
    qreal deviceratio = m_terminalDisplay->devicePixelRatio();
    deviceratio = deviceratio*2;
    QPixmap pixmap(m_terminalDisplay->size() * deviceratio);
    pixmap.setDevicePixelRatio(deviceratio);
    m_terminalDisplay->render(&pixmap);
    pixmap.save(fileName);
}

void QTermWidget::setLocked(bool enabled)
{
    this->setEnabled(!enabled);
    m_terminalDisplay->setLocked(enabled);
}

void QTermWidget::setDrawLineChars(bool drawLineChars)
{
    m_terminalDisplay->setDrawLineChars(drawLineChars);
}

void QTermWidget::setBoldIntense(bool boldIntense)
{
    m_terminalDisplay->setBoldIntense(boldIntense);
}

void QTermWidget::setConfirmMultilinePaste(bool confirmMultilinePaste) {
    m_terminalDisplay->setConfirmMultilinePaste(confirmMultilinePaste);
}

void QTermWidget::setTrimPastedTrailingNewlines(bool trimPastedTrailingNewlines) {
    m_terminalDisplay->setTrimPastedTrailingNewlines(trimPastedTrailingNewlines);
}

void QTermWidget::setEcho(bool echo) {
    m_echo = echo;
}

void QTermWidget::setKeyboardCursorColor(bool useForegroundColor, const QColor& color) {
    m_terminalDisplay->setKeyboardCursorColor(useForegroundColor, color);
}

void QTermWidget::addHighLightText(const QString &text, const QColor &color)
{
    for (int i = 0; i < m_highLightTexts.size(); i++) {
        if (m_highLightTexts.at(i)->text == text) {
            return;
        }
    }
    HighLightText *highLightText = new HighLightText(text,color);
    m_highLightTexts.append(highLightText);
    m_terminalDisplay->filterChain()->addFilter(highLightText->regExpFilter);
    m_terminalDisplay->updateFilters();
    m_terminalDisplay->repaint();
}

QMap<QString, QColor> QTermWidget::getHighLightTexts(void)
{
    QMap<QString, QColor> highLightTexts;
    for (int i = 0; i < m_highLightTexts.size(); i++) {
        highLightTexts.insert(m_highLightTexts.at(i)->text, m_highLightTexts.at(i)->color);
    }
    return highLightTexts;
}

bool QTermWidget::isContainHighLightText(const QString &text)
{
    for (int i = 0; i < m_highLightTexts.size(); i++) {
        if (m_highLightTexts.at(i)->text == text) {
            return true;
        }
    }
    return false;
}

void QTermWidget::removeHighLightText(const QString &text)
{
    for (int i = 0; i < m_highLightTexts.size(); i++) {
        if (m_highLightTexts.at(i)->text == text) {
            m_terminalDisplay->filterChain()->removeFilter(m_highLightTexts.at(i)->regExpFilter);
            delete m_highLightTexts.at(i);
            m_highLightTexts.removeAt(i);
            m_terminalDisplay->updateFilters();
            break;
        }
    }
    m_terminalDisplay->repaint();
}

void QTermWidget::clearHighLightTexts(void)
{
    for (int i = 0; i < m_highLightTexts.size(); i++) {
        m_terminalDisplay->filterChain()->removeFilter(m_highLightTexts.at(i)->regExpFilter);
        delete m_highLightTexts.at(i);
    }
    m_terminalDisplay->updateFilters();
    m_highLightTexts.clear();
    m_terminalDisplay->repaint();
}

void QTermWidget::setWordCharacters(const QString &wordCharacters)
{
    m_terminalDisplay->setWordCharacters(wordCharacters);
}

QString QTermWidget::wordCharacters(void) {
    return m_terminalDisplay->wordCharacters();
}

void QTermWidget::setShowResizeNotificationEnabled(bool enabled) {
    m_terminalDisplay->setShowResizeNotificationEnabled(enabled);
}

void QTermWidget::setEnableHandleCtrlC(bool enable) {
    m_session->emulation()->setEnableHandleCtrlC(enable);
}

int QTermWidget::lines() {
    return m_terminalDisplay->lines();
}

int QTermWidget::columns() {
    return m_terminalDisplay->columns();
}

int QTermWidget::getCursorX() {
    return m_terminalDisplay->getCursorX();
}

int QTermWidget::getCursorY() {
    return m_terminalDisplay->getCursorY();
}

void QTermWidget::setCursorX(int x) {
    m_terminalDisplay->setCursorX(x);
}

void QTermWidget::setCursorY(int y) {
    m_terminalDisplay->setCursorY(y);
}

QString QTermWidget::screenGet(int row1, int col1, int row2, int col2, int mode) {
    return m_terminalDisplay->screenGet(row1, col1, row2, col2, mode);
}

void QTermWidget::setSelectionOpacity(qreal opacity) {
    m_terminalDisplay->setSelectionOpacity(opacity);
}

void QTermWidget::setUrlFilterEnabled(bool enable) {
    if(m_UrlFilterEnable == enable) {
        return;
    }
    if(enable) {
        m_terminalDisplay->filterChain()->addFilter(urlFilter);
    } else {
        m_terminalDisplay->filterChain()->removeFilter(urlFilter);
    }
}

void QTermWidget::setMessageParentWidget(QWidget *parent) {
    messageParentWidget = parent;
    m_terminalDisplay->setMessageParentWidget(messageParentWidget);
}

void QTermWidget::reTranslateUi(void) {
    m_searchBar->retranslateUi();
}

void QTermWidget::set_fix_quardCRT_issue33(bool fix) {
    m_terminalDisplay->set_fix_quardCRT_issue33(fix);
}
