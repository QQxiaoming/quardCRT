<div style="text-align: right"><a href="../../en/latest/changelog.html">🇺🇸 English</a> | <a href="../../zh-cn/latest/changelog.html">🇨🇳 简体中文</a> | <a href="../../zh-tw/latest/changelog.html">🇭🇰 繁體中文</a> | <a href="../../ja/latest/changelog.html">🇯🇵 日本語</a></div>

# Change Log

## [[Unrelease](https://github.com/QQxiaoming/quardCRT)]

- Add Python scripting engine for scripting feature [#31](https://github.com/QQxiaoming/quardCRT/pull/31)
- Add the feature of selecting end-of-line sequence
- Add status bar log information, SSH ciphers information
- Add emphasized color border to the activated session in split screen mode
- Add custom color feature to the highlight
- Allow editing the text to be pasted when confirming multiple lines of paste
- Fix the issue that clicking the new tab button in split screen mode may not create the session correctly or be located under the wrong tab group
- Fix the issue that the SSH connection cannot be reconnected by tapping the Enter key in some cases
- Fix the issue that the target session object is inaccurate when locking/unlocking the session

## [[V0.4.8](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.4.8)] - 2024-07-26

- Add echo feature
- Fix the problem that some session types cannot reconnect
- Add the feature that the session in a non-connected state can automatically reconnect by clicking the Enter key
- Add the feature that the serial port automatically detects the physical connection disconnection
- Add a refresh serial port button in the serial port selection page
- Add up to four window split screen mode and multiple layout modes
- Add single/group/all session three modes in the send command window
- Add the plugin information page to display the plugin website
- Add notification center
- Add internal command window
- Improve the context menu of URL recognition links
- Improve the find window to automatically fill in the currently selected text each time it is opened
- Improve status bar
- Fix the Telnet session storage configuration error in non-English environments, which causes connection problems [#IAADHZ](https://gitee.com/QQxiaoming/quardCRT/issues/IAADHZ)
- Add pre-built plugins [timestamp](https://github.com/QuardCRT-platform/plugin-timestamp), and update pre-built plugin [ListSerial](https://github.com/QuardCRT-platform/plugin-ListSerial) to V0.0.3, update pre-built plugin [CharacterCode](https://github.com/QuardCRT-platform/plugin-CharacterCode) to V0.0.4 version

## [[V0.4.7](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.4.7)] - 2024-06-26

- Add broadcast session feature [#36](https://github.com/QQxiaoming/quardCRT/issues/36)
- Add label tag color feature
- Add block selection (Shift+click) and column selection (Alt+Shift+click) feature
- Add user-defined cursor color setting
- Add tooltip for optional items in settings
- Add advanced options to set the default startup shell for local terminal (default to $SHELL on Linux/MacOS, and default to the system built-in powershell on Windows, this function is only used to adjust the PowerShell version on Windows, not to start other shells, other shells please use LocalShell session to set specific commands)
- Fix the crash problem caused by modifying the configuration of the currently running session
- Fix the problem of incorrect pop-up position of the context menu in the case of multiple screens
- Modify to switch to the tab when right-clicking on a non-current tab
- Fix the cursor alignment error in the local shell on the windows msvc version, need to disable resizeQuirk [#39](https://github.com/QQxiaoming/quardCRT/issues/39)
- Fix the invalid problem of confirming multiple lines of text paste, and allow users to set enable/disable by themselves
- Fix the problem of automatically trimming empty lines in the pasted content of text, and allow users to set enable/disable by themselves
- Fix the unstable SSH initialization terminal size problem [#40](https://github.com/QQxiaoming/quardCRT/issues/40)
- Fix the probability crash problem after the SSH remote actively ends
- Fix font display abnormalities when restoring font settings to built-in fonts
- Fix the abnormal window position problem caused by window movement/resize operations on Linux with multiple screens
- Add pre-built plugins [TextStatistics](https://github.com/QuardCRT-platform/plugin-TextStatistics), and update pre-built plugin [CharacterCode](https://github.com/QuardCRT-platform/plugin-CharacterCode) to V0.0.3

## [[V0.4.6](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.4.6)] - 2024-05-26

- Add setting main window theme color feature
- Add status bar display session information feature
- Add WSL terminal toolbar button on Windows
- Add user-defined plugin loading path setting
- Fix the issue that the working directory is not correctly cloned when cloning the tab in some shell environments
- Fix the issue that the modification is saved when canceling the modification in the keyboard binding settings
- Fix the issue that the confirmation dialog cannot select cancel when closing all tabs
- Fix the issue that the session manager switches the current selected session incorrectly after modifying the session properties
- Fix the issue that the new session may be created on the hidden tab group
- Add pre-built plugins [CharacterCode](https://github.com/QuardCRT-platform/plugin-CharacterCode), [ListSerial](https://github.com/QuardCRT-platform/plugin-ListSerial), and update pre-built plugin [SearchOnWeb](https://github.com/QuardCRT-platform/plugin-SearchOnWeb) to V0.0.4 version

## [[V0.4.5](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.4.5)] - 2024-04-26

- Modify the terminal to select the text after the text is emphasized with a transparency of 50%, rather than the original 100%
- Fix the issue of abnormal width of rendering symbols '×' '÷' and '‖'
- Fix the issue of program crash in a small probability situation
- Fix cursor positioning issues [#I8RB90](https://gitee.com/QQxiaoming/quardCRT/issues/I8RB90)
- Remove the dependency on core5compat in Qt (Refer to the pr upstream of qtermwidget, but with modifications).
- Prohibit to look up the historical information with the middle mouse scrolling (Refer to the pr upstream of qtermwidget, but with modifications).
- Fix rendering issue with abnormal width of some symbols
- Add ANSI OSC52 sequence support
- Fix the issue that disconnecting the session will directly close the tab instead of disconnecting the session
- Fix the issue that some UI in the quick connection window does not refresh when switching languages
- Improve the session manager page width can be freely adjusted
- The version built with MSVC on Windows uses ConPty instead of WinPty, and the version built with Mingw continues to use WinPty
- Improve the behavior of selecting CJK characters when double-clicking
- Improve the display of the input method pre-edit area
- Add terminal color scheme color palette feature in settings
- Add the feature of switching terminal color scheme automatically when switching themes
- Add confirmation dialog when deleting a session
- Fix the issue that the context menu is too long to display completely and difficult to operate
- Fix the display abnormality caused by some theme switching on Windows
- Update pre-built plugin [onestep](https://github.com/QuardCRT-platform/plugin-onestep) to V0.0.3

## [[V0.4.4](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.4.4)] - 2024-03-26

- Add ascii send/receive, Kermit send/receive, xyzmodem send/receive features
- Fix the problem that the status indicated by the start tftp menu item may be incorrect
- Fix the problem that the program crashes when the terminal displays Unicode characters beyond the first plane
- Fix the problem that the terminal area rendering does not refresh after drag the window to the screen edge to trigger the window size adjustment on linux

## [[V0.4.3](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.4.3)] - 2024-02-26

- Add a tooltip to the terminal link, and modify the mouse shape to the corresponding shape after pressing ctrl
- Modify the default LC_CTYPE configuration to UTF-8 on macos, and do not make default configuration on other platforms. You can modify whether to make configuration through the setting file
- Fix the problem that the pre-built version on macos missed part of the packaged translation files
- Fix the problem that the terminal area rendering is not refreshed in time when the window is maximized on linux
- The plugin system completes multi-language support
- Partial UI details beautification
- Add a new help document
- Add German/Portuguese (Brazil)/Czech/Arabic support

## [[V0.4.2](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.4.2)] - 2024-01-28

- Add ESC to exit full screen mode in full screen mode
- The plugin platform supports dynamic enable/disable plugin features
- Optimize and solve the problem of fixed CPU load after software startup
- Optimize the problem of incorrect cursor position when searching up and down the history command in the local terminal under windows
- Fix the possible error ack processing problem of tftpsever
- Fix the crash problem under full screen on macos
- Fix the title button does not switch to the macos style problem in the native UI mode on macos
- Fix the problem that the title button full screen on macos through the native UI style cannot display the exit full screen option in the context menu on the main window
- Pre-built versions add pre-built plugin packaging for the [plugin ecosystem platform](https://github.com/QuardCRT-platform), including plugins [SearchOnWeb](https://github.com/QuardCRT-platform/plugin-SearchOnWeb), [onestep](https://github.com/QuardCRT-platform/plugin-onestep), [quickcomplete](https://github.com/QuardCRT-platform/plugin-quickcomplete) for the first time

## [[V0.4.1](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.4.1)] - 2024-01-13

- Add the feature of opening a session in a new window in the session manager
- Add the setting label tab plus button mode optional for new session/clone session/local shell session three ways
- Add filtering feature in session manager
- Fix the window flashing problem caused by the preview window when dragging the label

## [[V0.4.0](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.4.0)] - 2023-12-20

- The main UI is fully updated, the overall style is more modern, and the matching light/dark theme is more beautiful
- Add the feature of opening the current setting file in the setting option
- Add the experimental feature in the upper right corner, and the SSH scan is migrated to the laboratory, and the plugin interface is opened to add more specific features to the laboratory, visit [plugin platform](https://github.com/QuardCRT-platform)
- The SFTP window adds a bookmark feature and improves the SFTP operation logic.
- Improve and optimize the HexView window
- Improve the session cloning feature, the session type that can be cloned (SSH, Telent, etc.) is directly cloned, and the session type that cannot be cloned (serial port, etc.) will pop up the session setting window
- Improve the style when dragging the label, now the mouse can be correctly displayed as a grip gesture
- Fix the problem that the history page is not refreshed when opening the session properties
- Fix the rendering error of the line character in the session
- Fix the default terminal size is incorrect when opening the SSH session without changing the window size
- Fix the Tab Add New button style is not updated when switching the theme
- Fix the problem that the terminal color scheme cannot be persistently saved after modification [#I8PLTP](https://gitee.com/QQxiaoming/quardCRT/issues/I8PLTP)

## [[V0.3.1](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.3.1)] - 2023-12-05

- Add password input box support display/hide feature
- Add common baud rate prompt
- Add right-click menu request translation feature
- Add practical tools, scan local network SSH service
- SFTP file transfer window adds transfer progress display
- Add vnc protocol support
- Fix the problem that the port information of the serial port session cannot be correctly connected after being stored
- Fix the problem that the keychain authorization fails to use the wrong information
- Fix the problem that the maximum value of the port number input box is incorrect, resulting in the inability to correctly save the session information

## [[V0.3.0](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.3.0)] - 2023-11-28

- Add SSH2 protocol support (currently only supports password authentication, user passwords are stored in the local system keychain, not stored in the software configuration file)
- Add SSH2 session open SFTP file transfer window feature
- Add custom setting word characters feature, you can set which characters are considered as word characters, which is convenient for quickly selecting words
- Add double-click session management session to connect feature
- Add session manager close button
- Fix the problem that the session manager session sorting is rearranged after modifying the session properties
- Fix the problem that the serial port connection is normal but an error prompt "No Error" pops up
- Fix the problem that the session that failed to connect cannot be closed

## [[V0.2.6](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.6)] - 2023-11-15

- Add label tab hover preview feature
- Windows local terminal enhancement, now you can use the Tab key to complete the command like linux
- Add session status query, view session status through session settings/properties-status, currently only support local Shell session query process information
- Terminal content matching supports path matching, you can right-click to quickly open files/directories
- Add highlight matching content feature
- Fix the problem of rendering position error of Chinese full-width quotation marks
- Fix the problem that macOS incorrectly binds the copy and paste shortcut keys, causing the terminal to be unable to kill the process
- Add more language (Spanish/French/Korean/Russian/Traditional Chinese) support (provided by GitHub Copilot)

## [[V0.2.5](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.5)] - 2023-11-09

- Add floating window mode, and support moving back to the main window
- Add tab page free drag (drag to floating window or drag to split screen)
- Fix the serious crash problem caused by moving the label/label title display error problem
- Add advanced settings to select the UI style as the native style (single platform users may want to use the native style, multi-platform users may want to use the unified style)
- Add one-click cleaning of selected background images in settings
- Optimize the secondary confirmation feature when exiting the application/closing the session, now the local Shell session will decide whether to require secondary confirmation based on whether there are child processes, and other session types remain unchanged
- Add setting label title mode, you can choose to display the session title in brief/complete/scrolling, three ways

## [[V0.2.4](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.4)] - 2023-11-03

- Add windows NamedPipe protocol support (unix domain socket corresponding to linux/macos)
- Fix the problem that the printer service cannot be started on windows
- Fix the problem that part of the memory is not released when closing the tab
- Add secondary confirmation feature when exiting the application/closing the session
- Add session link status display feature (the tab displays in the form of an icon)
- Add terminal font setting feature
- Add terminal scroll line number setting
- Add setting cursor shape and blink feature
- Terminal background supports Gif animation format files (need to enable animation support in advanced settings)
- Terminal background supports mp4/avi/mkv/mov video format files (need to enable animation support in advanced settings)
- Update the global settings window, categorize the settings
- Add session settings window, now you can modify and edit session properties
- Add separate start new window feature
- Add software's own debugging information log system, which is not turned on by default, and must be manually written to the config file to start

## [[V0.2.3](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.3)] - 2023-10-26

- Fix the error ci environment, no substantial changes from V0.2.2

## [[V0.2.2](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.2)] - 2023-10-26

- Fix the session information storage error that caused the right session manager to fail to connect correctly [#I8AJN1](https://gitee.com/QQxiaoming/quardCRT/issues/I8AJN1)
- Quick connection supports selecting only open session and only save session
- Improve bookmark management feature
- Implement save settings and real-time save settings button feature
- Add ALT+'-' and ALT+'=' global shortcut keys to switch the current tab
- Add ALT+'{num}' global shortcut key to switch to the specified tab
- Add ALT+LEFT, ALT+RIGHT global shortcut keys mapped to home and end keys (considering that macbook does not have home and end keys)
- Add high-definition screenshot current terminal feature
- Add export current terminal session content (pdf/txt) feature
- Add printer print current terminal session feature
- Add lock session feature
- Add configuration to start a new local terminal with the default working path
- Terminal background image supports tile mode
- Persistently store the main window layout information
- Optimize the context menu on the main window
- Optimize the open file dialog UI
- Modify the rendering bold font on windows to cause the cursor to be abnormal, temporarily not supporting bold rendering to ensure that the cursor position is correct
- Fix the problem that the working path set after executing the command on windows is incorrect
- Add cmd+delete as delete key feature on macos (because the delete key is backspace on macos)

## [[V0.2.1](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.1)] - 2023-10-19

- linux package fcit plugin to support more Chinese input methods
- Fix the rendering cursor error of Chinese characters on linux
- Fix HEX view format rendering error and data acquisition failure problem
- Persistently store session information

## [[V0.2.0](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.0)] - 2023-10-18

- Add the feature of switching simple/standard UI with alt+n/alt+J
- Add the practical feature of starting tftp server
- Optimize hex view display window
- The window installation program adds the system-integrated right-click menu opened with quardCRT
- The tab bar right-click menu adds multiple practical features
- Fix the position error of wide characters such as Chinese on window
- Add terminal background image configuration feature
- Add directory bookmark feature
- Add persistent storage of user settings feature
- Fix some UI details error
- Fix some small probability of possible crash problems

## [[V0.1.5](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.5)] - 2023-10-09

- Add terminal right scroll bar
- Fix the problem that the middle mouse button cannot execute copy and paste on window/mac
- Add dual terminal session split display
- Add simple UI options

## [[V0.1.4](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.4)] - 2023-10-08

- Add sidebar session manager
- Add status bar prompt information
- Add command input bar
- Improve shortcut key feature
- Fix the problem that the session clone has inconsistent working paths
- Add dynamic parsing of terminal URL content
- Fix possible crash problems

## [[V0.1.3](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.3)] - 2023-09-29

- Re-fix the problem that windows cannot start (introduced in V0.1.0) (this version completely fixes the problem introduced in V0.1.0)

## [[V0.1.2](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.2)] - 2023-09-29

- Re-fix the problem that windows cannot start (introduced in V0.1.0)

## [[V0.1.1](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.1)] - 2023-09-29

- Fix the problem that windows cannot start (introduced in V0.1.0)

## [[V0.1.0](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.0)] - 2023-09-29

- Add the feature of switching long/short names by double-clicking the label tab name
- Add storage log feature
- Add quick open local Shell shortcut key and clone session shortcut key
- Add windows to perceive the current real-time working path (the same experience as linux and mac)
- Add HEX viewer

## [[V0.0.2](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.0.2)] - 2023-09-27

- Add menu bar and toolbar to the main window, and improve more right-click menus
- Add more terminal theme styles
- Add dynamic switching language feature
- Add dynamic switching theme feature

## [[V0.0.1](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.0.1)] - 2023-09-26

- Realize the main window terminal feature
- Add label tab paging management
- Support telnet protocol
- Support serial port protocol
- Support RAW protocol
- Support local Shell
- UI supports simplified Chinese/English/Japanese
