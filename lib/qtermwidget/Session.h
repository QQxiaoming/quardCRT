/*
    This file is part of Konsole, an X terminal.

    Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>
    Copyright (C) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>, Copyright (C)2008

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

#ifndef SESSION_H
#define SESSION_H

#include <QStringList>
#include <QWidget>

#include "Emulation.h"
#include "History.h"

class Emulation;
class TerminalDisplay;

/**
 * Represents a terminal session consisting of a pseudo-teletype and a terminal emulation.
 * The pseudo-teletype (or PTY) handles I/O between the terminal process and Konsole.
 * The terminal emulation ( Emulation and subclasses ) processes the output stream from the
 * PTY and produces a character image which is then shown on views connected to the session.
 *
 * Each Session can be connected to one or more views by using the addView() method.
 * The attached views can then display output from the program running in the terminal
 * or send input to the program in the terminal in the form of keypresses and mouse
 * activity.
 */
class Session : public QObject {
    Q_OBJECT

public:
    Q_PROPERTY(QString keyBindings READ keyBindings WRITE setKeyBindings)
    Q_PROPERTY(QSize size READ size WRITE setSize)

    /**
     * Constructs a new session.
     */
    Session(QObject* parent = nullptr);
    ~Session() override;

    /**
     * Sets the profile associated with this session.
     *
     * @param profileKey A key which can be used to obtain the current
     * profile settings from the SessionManager
     */
    void setProfileKey(const QString & profileKey);
    /**
     * Returns the profile key associated with this session.
     * This can be passed to the SessionManager to obtain the current
     * profile settings.
     */
    QString profileKey() const;

    /**
     * Adds a new view for this session.
     *
     * The viewing widget will display the output from the terminal and
     * input from the viewing widget (key presses, mouse activity etc.)
     * will be sent to the terminal.
     *
     * Views can be removed using removeView().  The session is automatically
     * closed when the last view is removed.
     */
    void addView(TerminalDisplay * widget);
    /**
     * Removes a view from this session.  When the last view is removed,
     * the session will be closed automatically.
     *
     * @p widget will no longer display output from or send input
     * to the terminal
     */
    void removeView(TerminalDisplay * widget);

    /**
     * Returns the views connected to this session
     */
    QList<TerminalDisplay *> views() const;

    /**
     * Returns the terminal emulation instance being used to encode / decode
     * characters to / from the process.
     */
    Emulation * emulation() const;

    /** Returns the unique ID for this session. */
    int sessionId() const;

    /**
     * Return the session title set by the user (ie. the program running
     * in the terminal), or an empty string if the user has not set a custom title
     */
    QString userTitle() const;

    /**
     * Sets the type of history store used by this session.
     * Lines of output produced by the terminal are added
     * to the history store.  The type of history store
     * used affects the number of lines which can be
     * remembered before they are lost and the storage
     * (in memory, on-disk etc.) used.
     */
    void setHistoryType(const HistoryType & type);
    /**
     * Returns the type of history store used by this session.
     */
    const HistoryType & historyType() const;
    /**
     * Clears the history store used by this session.
     */
    void clearHistory();

    /**
     * Enables monitoring for activity in the session.
     * This will cause notifySessionState() to be emitted
     * with the NOTIFYACTIVITY state flag when output is
     * received from the terminal.
     */
    void setMonitorActivity(bool);
    /** Returns true if monitoring for activity is enabled. */
    bool isMonitorActivity() const;

    /**
     * Enables monitoring for silence in the session.
     * This will cause notifySessionState() to be emitted
     * with the NOTIFYSILENCE state flag when output is not
     * received from the terminal for a certain period of
     * time, specified with setMonitorSilenceSeconds()
     */
    void setMonitorSilence(bool);
    /**
     * Returns true if monitoring for inactivity (silence)
     * in the session is enabled.
     */
    bool isMonitorSilence()  const;
    /** See setMonitorSilence() */
    void setMonitorSilenceSeconds(int seconds);

    /**
     * Sets the key bindings used by this session.  The bindings
     * specify how input key sequences are translated into
     * the character stream which is sent to the terminal.
     *
     * @param id The name of the key bindings to use.  The
     * names of available key bindings can be determined using the
     * KeyboardTranslatorManager class.
     */
    void setKeyBindings(const QString & id);
    /** Returns the name of the key bindings used by this session. */
    QString keyBindings() const;

    /**
     * Sets whether flow control is enabled for this terminal
     * session.
     */
    void setFlowControlEnabled(bool enabled);

    /** Returns whether flow control is enabled for this terminal session. */
    bool flowControlEnabled() const;

    /**
     * Sends @p text to the current foreground terminal program.
     */
    void sendText(const QString & text) const;

    void sendKeyEvent(QKeyEvent* e) const;

    /** Returns the terminal session's window size in lines and columns. */
    QSize size();
    /**
     * Emits a request to resize the session to accommodate
     * the specified window size.
     *
     * @param size The size in lines and columns to request.
     */
    void setSize(const QSize & size);

    /** Sets the text codec used by this session's terminal emulation. */
    void setCodec(QStringEncoder codec) const;

    /**
     * Sets whether the session has a dark background or not.  The session
     * uses this information to set the COLORFGBG variable in the process's
     * environment, which allows the programs running in the terminal to determine
     * whether the background is light or dark and use appropriate colors by default.
     *
     * This has no effect once the session is running.
     */
    void setDarkBackground(bool darkBackground);
    /**
     * Returns true if the session has a dark background.
     * See setDarkBackground()
     */
    bool hasDarkBackground() const;

    /**
     * Attempts to get the shell program to redraw the current display area.
     * This can be used after clearing the screen, for example, to get the
     * shell to redraw the prompt line.
     */
    void refresh();

//  void startZModem(const QString &rz, const QString &dir, const QStringList &list);
//  void cancelZModem();
//  bool isZModemBusy() { return _zmodemBusy; }

    /**
     * Returns a pty slave file descriptor.
     * This can be used for display and control
     * a remote terminal.
     */
    int recvData(const char *buff, int len);

    // Returns true if the current screen is the secondary/alternate one
    // or false if it's the primary/normal buffer
    bool isPrimaryScreen();

public slots:
    /**
     * Starts the terminal session for "as is" PTY
     * (without the direction a data to internal terminal process).
     * It can be used for control or display a remote/external terminal.
     */
    void runEmptyPTY();

    /**
     * Closes the terminal session.  This sends a hangup signal
     * (SIGHUP) to the terminal process and causes the done(Session*)
     * signal to be emitted.
     */
    void close();

    /**
     * Changes the session title or other customizable aspects of the terminal
     * emulation display. For a list of what may be changed see the
     * Emulation::titleChanged() signal.
     */
    void setUserTitle( int, const QString & caption );

signals:
    /** Emitted when the terminal process starts. */
    void started();

    /**
     * Emitted when the terminal process exits.
     */
    void finished();

    /**
     * Emitted when output is received from the terminal process.
     */
    void receivedData( const QString & text );

    /** Emitted when the session's title has changed. */
    void titleChanged(int title,const QString& newTitle);

    /** Emitted when the session's profile has changed. */
    void profileChanged(const QString & profile);

    /**
     * Emitted when the activity state of this session changes.
     *
     * @param state The new state of the session.  This may be one
     * of NOTIFYNORMAL, NOTIFYSILENCE or NOTIFYACTIVITY
     */
    void stateChanged(int state);

    /** Emitted when a bell event occurs in the session. */
    void bellRequest( const QString & message );

    /**
     * Requests that the color the text for any tabs associated with
     * this session should be changed;
     *
     * TODO: Document what the parameter does
     */
    void changeTabTextColorRequest(int);

    /**
     * Requests that the background color of views on this session
     * should be changed.
     */
    void changeBackgroundColorRequest(const QColor &);

    /** TODO: Document me. */
    void openUrlRequest(const QString & url);

    /**
     * Emitted when the terminal process requests a change
     * in the size of the terminal window.
     *
     * @param size The requested window size in terms of lines and columns.
     */
    void resizeRequest(const QSize & size);

    /**
     * Emitted when a profile change command is received from the terminal.
     *
     * @param text The text of the command.  This is a string of the form
     * "PropertyName=Value;PropertyName=Value ..."
     */
    void profileChangeCommandReceived(const QString & text);

    /**
     * Emitted when the flow control state changes.
     *
     * @param enabled True if flow control is enabled or false otherwise.
     */
    void flowControlEnabledChanged(bool enabled);

    /**
     * Emitted when the active screen is switched, to indicate whether the primary
     * screen is in use.
     *
     * This signal serves as a relayer of Emulation::priamyScreenInUse(bool),
     * making it usable for higher level component.
     */
    void primaryScreenInUse(bool use);

    /**
     * Broker for Emulation::cursorChanged() signal
     */
    void cursorChanged(Emulation::KeyboardCursorShape cursorShape, bool blinkingCursorEnabled);

    void silence();
    void activity();

private slots:
    void onReceiveBlock( const char * buffer, int len );
    void monitorTimerDone();
    void activityStateSet(int);
    //automatically detach views from sessions when view is destroyed
    void viewDestroyed(QObject * view);

private:
    void updateTerminalSize();

    int            _uniqueIdentifier;

    Emulation  *  _emulation;

    QList<TerminalDisplay *> _views;

    bool           _monitorActivity;
    bool           _monitorSilence;
    bool           _notifiedActivity;
    bool           _masterMode;
    bool           _wantedClose;
    QTimer    *    _monitorTimer;
    int            _silenceSeconds;
    QString        _nameTitle;
    QString        _userTitle;
    QString        _iconName;
    QString        _iconText; // as set by: echo -en '\033]1;IconText\007
    bool           _isTitleChanged; ///< flag if the title/icon was changed by user
    bool           _flowControl;
    int            _sessionId;
    // Color/Font Changes by ESC Sequences
    QColor         _modifiedBackground; // as set by: echo -en '\033]11;Color\007
    bool           _hasDarkBackground;
    bool           _isPrimaryScreen;
    static int     lastSessionId;
};

#endif
