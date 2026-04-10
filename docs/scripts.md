<div style="text-align: right"><a href="../../en/latest/scripts.html">🇺🇸 English</a> | <a href="../../zh-cn/latest/scripts.html">🇨🇳 简体中文</a> | <a href="../../zh-tw/latest/scripts.html">🇭🇰 繁體中文</a> | <a href="../../ja/latest/scripts.html">🇯🇵 日本語</a></div>

# Scripts

quardCRT supports scripting from version V0.5.0. Scripts are written in Python and can call quardCRT's built-in API to automate terminal actions, dialog interaction, screen matching, and file transfer workflows.

This page is organized as a practical user guide first, followed by the API reference.

## What scripts are useful for

Typical uses include:

- logging in to a device or server with a repeatable sequence
- waiting for specific prompts before sending the next command
- automating test or initialization steps
- launching transfers or small workflow helpers from inside quardCRT
- generating a starter script by recording a manual session

## Availability

Script support depends on builds that include Python support. If your build does not include the Python runtime integration, script menu actions may be disabled.

## Run a script

To run a Python script from the UI:

1. Open the `Script` menu.
2. Select `Run...`.
3. Choose a Python script file.
4. quardCRT starts the script and disables `Run...` until the current script finishes or is cancelled.

When you run a script successfully from the file picker, quardCRT also adds it to the recent script list so it can be launched again directly from the same menu.

## Cancel a running script

If a script is currently running, use the `Script` menu's cancel action to stop execution.

This is useful when:

- the remote side is no longer responding as expected
- you launched the wrong script
- the script is waiting longer than intended

## Record a script

quardCRT can record interactive terminal activity and generate a starter Python script.

From the `Script` menu:

1. Select `Start Recording Script`.
2. Interact with the current terminal session.
3. Select `Stop Recording Script...` to save the generated `.py` file.

If you decide not to keep the recording, use `Cancel Recording Script` instead.

### How recorded scripts are generated

The generated file is a Python script that typically contains:

- `crt.Screen.Synchronous = True`
- `crt.Screen.Send(...)` for captured user input
- `crt.Screen.WaitForString(...)` for captured received prompts or output boundaries

This gives you a useful starting point, but recorded scripts are not guaranteed to be production-ready. In real use, you often need to clean up prompts, add better waiting logic, or remove unnecessary steps.

## Recent scripts

After running a script from the file picker, quardCRT stores it in the recent script list under the `Script` menu.

If you want to clear that list, use `Clean all recent script`.

## Example scripts in this repository

The repository also includes small example scripts under `test/scriptengine/` that you can run and modify.

Useful starting points include:

- `test/scriptengine/session/prompted_ssh2.py`: prompt for host, port, username, and password, then connect with SSH2
- `test/scriptengine/session/prompted_telnet_login.py`: connect with Telnet and complete a login sequence by waiting for login, password, and shell prompts
- `test/scriptengine/screen/send_command_and_capture.py`: send a command to the active session and read output until a prompt is matched
- `test/scriptengine/screen/save_screen_to_file.py`: save the current visible screen text to a local file
- `test/scriptengine/misc/repeat_command_logger.py`: run the same command multiple times and save each round of output to a log file
- `test/scriptengine/tab/send_to_all_sessions.py`: send the same command to every session in the active tab group
- `test/scriptengine/filetransfer/zmodem_upload_dialog.py`: select a local file and start a Zmodem upload

## First example

Here is a minimal script that displays the quardCRT version in a message box:

```python
import sys
from quardCRT import crt

def main():
    # Display quardCRT's version
    crt.Dialog.MessageBox("quardCRT version is: " + crt.Version)

if __name__ == '__main__':
    main()
```

This script is still a normal Python file. The main difference is that quardCRT injects the `quardCRT` module and its automation objects.

What the example does:

- `import sys`: Import the `sys` module to get command line arguments.
- `from quardCRT import crt`: Import the API of quardCRT.
- `def main():`: Define a `main` function to execute the main logic of the script.
- `# Display quardCRT's version`: Comment to explain the next line of code.
- `crt.Dialog.MessageBox("quardCRT version is: " + crt.Version)`: Call the API of quardCRT to display a message box showing the version information of quardCRT.
- `if __name__ == '__main__':`: Determine whether the script is run as the main program.
- `main()`: Call the `main` function to execute the main logic of the script.

## A practical automation pattern

For terminal automation, a common pattern is:

1. Get the active screen or session.
2. Send a command with `crt.Screen.Send(...)`.
3. Wait for the next expected prompt with `crt.Screen.WaitForString(...)` or `crt.Screen.WaitForStrings(...)`.
4. Repeat until the workflow is finished.

This pattern is usually more reliable than blindly sleeping for fixed time intervals.

## API overview

The API of quardCRT includes the following parts:

- `crt`: The main API of quardCRT.
- `crt.Dialog`: Used to display dialog boxes.
- `crt.Session`: Used to manage the currently active session.
- `crt.Screen`: Used to manage the currently active screen.
- `crt.Window`: Used to manage the window of quardCRT.
- `crt.Arguments`: Used to get command line arguments.
- `crt.Clipboard`: Used to operate the clipboard.
- `crt.FileTransfer`: Used to operate file transfer.
- `crt.CommandWindow`: Used to operate the command window.
- `crt.Tab`: Used to manage the tab group.

The remainder of this page is a reference for these objects.

### crt

`crt` is the main API of quardCRT, including the following parts:

#### Method

- `crt.GetActiveTab() -> object`：Get the currently active tab group.
    - Return value: Tab object.
    - Example:

        ```python
        tab = crt.GetActiveTab()
        ```
    - Note: If there is no active Tab group, get the first Tab group.

- `crt.GetLastError() -> object`：Get the last error that occurred.
    - Return value: Error object.
    - Example:

        ```python
        error = crt.GetLastError()
        ```

- `crt.GetLastErrorMessage() -> str`：Get the error message of the last error that occurred.
    - Return value: Error message.
    - Example:

        ```python
        message = crt.GetLastErrorMessage()
        ```

- `crt.ClearLastError()`: Clear the last error that occurred.
    - Example:

        ```python
        crt.ClearLastError()
        ```

- `crt.Sleep(milliseconds: int)`: Pause the execution of the script.
    - Parameter:
        - `milliseconds`: The time to pause, in milliseconds.
    - Example:

        ```python
        crt.Sleep(1000)
        ```

- `crt.Cmd(cmd: str, [args:list]) -> str`：Execute a special command of quardCRT.
    - Parameter:
        - `cmd`: Command name.
        - `args`: Command parameter list.
    - Return value: Command execution result.
    - Note: Please refer to the command help of quardCRT for the command name and parameter list.
    - Example:

        ```python
        result = crt.Cmd("show", ["version"])
        ```

- `crt.Quit()`: Exit quardCRT.
    - Example:

        ```python
        crt.Quit()
        ```

#### Attribute

- `crt.Dialog`: Global dialog object.
- `crt.Session`: Current session object.
- `crt.Screen`: Current screen object.
- `crt.Window`: Global window object.
- `crt.Arguments`: Command line parameter object.
- `crt.Clipboard`: Clipboard object.
- `crt.FileTransfer`: File transfer object.
- `crt.ScriptFullName`: The full path of the current script. Read-only.
- `crt.ActivePrinter`: The name of the currently active printer.
- `crt.Version`: The version information of quardCRT. Read-only.

### Dialog

`Dialog` is used to display dialog boxes, global singleton, called through the crt object, for example:

```python
dialog = crt.Dialog
```

It includes the following parts:

#### Method

- `Dialog.MessageBox(message: str, [title: str, buttons: int]) -> int`：Display a message box.
    - Parameter:
        - `message`: Message content.
        - `title`: Message title.
        - `buttons`: Button type.
    - Return value: The execution result. 0 means normal, other values mean abnormal.
    - Note: Button types include `Dialog.OK`, `Dialog.OK | Dialog.Cancel`, `Dialog.Abort | Dialog.Retry | Dialog.Ignore`, `Dialog.Yes | Dialog.No`, etc.
    - Example:

        ```python
        result = crt.Dialog.MessageBox("Hello, quardCRT!", "Message", crt.Dialog.OK)
        ```

- `Dialog.Prompt(prompt: str, name: str, input: str, password: bool) -> str`：Display an input box.
    - Parameter:
        - `prompt`: Prompt content.
        - `name`: Input box name.
        - `input`: Input box default value.
        - `password`: Whether it is a password input.
    - Return value: The value of the input box. If the user clicks Cancel, an empty string is returned.
    - Example:

        ```python
        value = crt.Dialog.Prompt("Please input your name:", "Name", "", False)
        password = crt.Dialog.Prompt("Please input your password:", "Password", "", True)
        ```

- `Dialog.FileOpenDialog(title: str, [buttonLabel: str, directory: str, filter: str]) -> str`：Display an open file dialog.
    - Parameter:
        - `title`: Dialog title.
        - `buttonLabel`: Button label.
        - `directory`: Default directory.
        - `filter`: File filter.
    - Return value: The selected file path. If the user clicks Cancel, an empty string is returned.
    - Example:

        ```python
        file = crt.Dialog.FileOpenDialog("Open File", "Open", "", "All Files (*.*)|*.*")
        ```

- `Dialog.FileSaveDialog(title: str, [buttonLabel: str, directory: str, filter: str]) -> str`：Display a save file dialog.
    - Parameter:
        - `title`: Dialog title.
        - `buttonLabel`: Button label.
        - `directory`: Default directory.
        - `filter`: File filter.
    - Return value: The selected file path. If the user clicks Cancel, an empty string is returned.
    - Example:

        ```python
        file = crt.Dialog.FileSaveDialog("Save File", "Save", "", "All Files (*.*)|*.*")
        ```

#### Attribute

- `Dialog.OK`: OK button. Read-only.
- `Dialog.Cancel`: Cancel button. Read-only.
- `Dialog.Abort`: Abort button. Read-only.
- `Dialog.Retry`: Retry button. Read-only.
- `Dialog.Ignore`: Ignore button. Read-only.
- `Dialog.Yes`: Yes button. Read-only.
- `Dialog.No`: No button. Read-only.

### Session

`Session` is used to manage sessions, called through the crt object or obtained through Tab, for example:

```python
session = crt.Session # Get the current session
```

```python
tab = crt.GetActiveTab()    # Get the currently active tab group
if tab.Number > 0:
    session = tab.GetSession(0) # Get the first session of the tab group
```

It includes the following parts:

#### Method

- `Session.Connect(cmd: str) -> int`：Connect to a host.
    - Parameter:
        - `cmd`: Command.
    - Return value: Connection result.
    - Note: The command format is `-<type> <arg>`, for example:
        - `-telnet <hostname> <port>`
        - `-serial <baudRate> <dataBits> <parity> <stopBits> <flowControl> <xEnable>`
        - `-localshell <path>`
        - `-raw <hostname> <port>`
        - `-namepipe <pipeName>`
        - `-ssh2 <hostname> <port> <username> <password>`
        - `-vnc <hostname> <port> <password>`
        - `-s <sessionName>`
        - `-clone`
    - Example:

        ```python
        result = session.Connect("-ssh2 example.com 22 root 123456")
        ```

- `Session.Disconnect()`: Disconnect the current session.
    - Example:

        ```python
        session.Disconnect()
        ```

- `Session.Log(enable: bool)`: Enable or disable logging.
    - Parameter:
        - `enable`: Whether to enable logging.
    - Example:

        ```python
        session.Log(True)
        ```

- `Session.Lock(prompt: str, password: str, lockallsessions: int) -> int`：Lock the session.
    - Parameter:
        - `prompt`: Prompt content.
        - `password`: Password.
        - `lockallsessions`: Whether to lock all sessions.
    - Return value: Execution result.
    - Example:

        ```python
        result = session.Lock("Please input your password:", "password", 0)
        ```

- `Session.Unlock(prompt: str, password: str, lockallsessions: int) -> int`：Unlock the session.
    - Parameter:
        - `prompt`: Prompt content.
        - `password`: Password.
        - `lockallsessions`: Whether to unlock all sessions.
    - Return value: Execution result.
    - Example:

        ```python
        result = session.Unlock("Please input your password:", "password", 0)
        ```

#### Attribute

- `Session.Connected`: Whether the session is connected. Read-only.
- `Session.Locked`: Whether the session is locked. Read-only.
- `Session.Logging`: Whether the session is logging. Read-only.
- `Session.Id`: The global ID of the session. Read-only.

### Screen

`Screen` is used to manage the screen, called through the crt object or obtained through Tab, for example:

```python
screen = crt.Screen # Get the current screen
```

```python
tab = crt.GetActiveTab()    # Get the currently active tab group
if tab.Number > 0:
    screen = tab.GetScreen(0) # Get the first screen of the tab group
```

It includes the following parts:

- `Screen.WaitForString(str: str, timeout: int, bcaseInsensitive: bool) -> str`：Wait for the specified text to appear on the screen.
    - Parameter:
        - `str`: The specified text.
        - `timeout`: Timeout time.
        - `bcaseInsensitive`: Whether to ignore case.
    - Return value: The matched text.
    - Example:

        ```python
        text = screen.WaitForString("Hello, quardCRT!", 1000, False)
        ```

- `Screen.WaitForStrings(strlist: list[str], timeout: int, bcaseInsensitive: bool) -> str`：Wait for the specified text list to appear on the screen.
    - Parameter:
        - `strlist`: The specified text list.
        - `timeout`: Timeout time.
        - `bcaseInsensitive`: Whether to ignore case.
    - Return value: The matched text.
    - Example:

        ```python
        text = screen.WaitForStrings(["Hello", "quardCRT"], 1000, False)
        ```

- `Screen.Send(str: str) -> int`：Send text to the screen.
    - Parameter:
        - `str`: The text to send.
    - Return value: The number of characters sent.
    - Example:

        ```python
        count = screen.Send("Hello, quardCRT!")
        ```

- `Screen.Clear()`: Clear the screen.
    - Example:

        ```python
        screen.Clear()
        ```

- `Screen.Get(row1: int, col1: int, row2: int, col2: int) -> str`：Get the text of the specified area of the screen.
    - Parameter:
        - `row1`: Starting row.
        - `col1`: Starting column.
        - `row2`: Ending row.
        - `col2`: Ending column.
    - Return value: The text of the specified area.
    - Example:

        ```python
        text = screen.Get(1, 1, 10, 80)
        ```

- `Screen.Get2(row1: int, col1: int, row2: int, col2: int) -> str`：Get the text of the specified area of the screen, insert line break symbols.
    - Parameter:
        - `row1`: Starting row.
        - `col1`: Starting column.
        - `row2`: Ending row.
        - `col2`: Ending column.
    - Return value: The text of the specified area.
    - Example:

        ```python
        text = screen.Get2(1, 1, 10, 80)
        ```

- `Screen.IgnoreCase(enable: bool)`: Enable or disable case-insensitive.
    - Parameter:
        - `enable`: Whether to enable case-insensitive.
    - Example:

        ```python
        screen.IgnoreCase(True)
        ```

- `Screen.Print()`: Print the screen content.
    - Example:

        ```python
        screen.Print()
        ```

- `Screen.Shortcut(path: str)`: Screenshot.
    - Parameter:
        - `path`: The path to save the screenshot.
    - Example:

        ```python
        screen.Shortcut("C:\\screenshot.png")
        ```

- `Screen.SendKeys(keylist: list[str])`: Send a combination of keys.
    - Parameter:
        - `keylist`: Key list.
    - Example:

        ```python
        screen.SendKeys(["Ctrl", "Alt", "Del"])
        ```

- `Screen.ReadString(strlist: list[str], timeout: int, bcaseInsensitive: bool) -> str`：Read text data until the specified text list appears on the screen.
    - Parameter:
        - `strlist`: The specified text list.
        - `timeout`: Timeout time.
        - `bcaseInsensitive`: Whether to ignore case.
    - Return value: The read text.
    - Example:

        ```python
        text = screen.ReadString(["Hello", "quardCRT"], 1000, False)
        ```

- `Screen.WaitForCursor(row: int, col: int, timeout: int) -> bool`：Wait for the cursor to move to the specified position. (Not implemented yet)
    - Parameter:
        - `row`: Specified row.
        - `col`: Specified column.
        - `timeout`: Timeout time.
    - Return value: Whether it is successful.
    - Example:

        ```python
        result = screen.WaitForCursor(10, 10, 1000)
        ```

- `Screen.WaitForKey(keylist: list[str], timeout: int) -> bool`：Wait for key input. (Not implemented yet)
    - Parameter:
        - `keylist`: Key list.
        - `timeout`: Timeout time.
    - Return value: Whether it is successful.
    - Example:

        ```python
        result = screen.WaitForKey(["Enter"], 1000)
        ```

#### Attribute

- `Screen.Synchronous`: Synchronous mode.
- `Screen.CurrentColumn`: The current cursor column. Read-only.
- `Screen.CurrentRow`: The current cursor row. Read-only.
- `Screen.Rows`: The number of screen rows. Read-only.
- `Screen.Columns`: The number of screen columns. Read-only.
- `Screen.IgnoreCase`: Whether to ignore case.
- `Screen.MatchIndex`: The matched index. Read-only.
- `Screen.Selection`: The selected text. Read-only.
- `Screen.Id`: The global ID of the screen. Read-only.

### Window

`Window` is used to manage the application window, global singleton, called through the crt object, for example:

```python
window = crt.Window
```

It includes the following parts:

#### Method


- `Window.Activate()`: Activate the window.
    - Example:

        ```python
        window.Activate()
        ```

- `Window.Show(type: int)`: Show the window.
    - Parameter:
        - `type`: Window display type.
    - Example:

        ```python
        window.Show(crt.Window.ShowNormal)
        ```

#### Attribute

- `Window.State`: The current window display type.
- `Window.Active`: Whether the current window is active.
- `Window.Hide`: Hide the window. Read-only.
- `Window.ShowNormal`: Show the window normally. Read-only.
- `Window.ShowMinimized`: Minimize the window. Read-only.
- `Window.ShowMaximized`: Maximize the window. Read-only.

### Arguments

`Arguments` is used to get command line parameters, global singleton, called through the crt object, for example:

```python
arguments = crt.Arguments
```

It includes the following parts:

#### Method

- `Arguments.GetArg(index: int) -> str`：Get the parameter of the specified index.
    - Parameter:
        - `index`: Parameter index.
    - Return value: Parameter value.
    - Example:

        ```python
        arg = arguments.GetArg(0)
        ```

#### Attribute

- `Arguments.Count`: The number of parameters. Read-only.

### Clipboard

`Clipboard` is used to operate the clipboard, global singleton, called through the crt object, for example:

```python
clipboard = crt.Clipboard
```

It includes the following parts:

#### Attribute

- `Clipboard.Text`: Clipboard text. Read-only.

### FileTransfer

`FileTransfer` is used to operate file transfer, global singleton, called through the crt object, for example:

```python
filetransfer = crt.FileTransfer
```

It includes the following parts:

#### Method

- `FileTransfer.AddToUploadList(path: str)`: Add a file to the zmodem upload list.
    - Parameter:
        - `path`: File path.
    - Example:

        ```python
        filetransfer.AddToUploadList("C:\\example.txt")
        ```

- `FileTransfer.ClearUploadList()`: Clear the zmodem upload list.
    - Example:

        ```python
        filetransfer.ClearUploadList()
        ```

- `FileTransfer.ReceiveKermit() -> int`：Receive kermit file.
    - Return value: Whether it is successful.
    - Example:

        ```python
        result = filetransfer.ReceiveKermit()
        ```

- `Filetransfer.SendKermit(path: str) -> int`：Send kermit file.
    - Parameter:
        - `path`: File path.
    - Return value: Whether it is successful.
    - Example:

        ```python
        result = filetransfer.SendKermit("C:\\example.txt")
        ```

- `FileTransfer.ReceiveXmodem(path: str)`：Receive xmodem file.
    - Parameter:
        - `path`: File path.
    - Return value: Whether it is successful.
    - Example:

        ```python
        result = filetransfer.ReceiveXmodem("C:\\example.txt")
        ```

- `FileTransfer.SendXmodem(path: str) -> int`：Send xmodem file.
    - Parameter:
        - `path`: File path.
    - Return value: Whether it is successful.
    - Example:

        ```python
        result = filetransfer.SendXmodem("C:\\example.txt")
        ```

- `FileTransfer.ReceiveYmodem() -> int`：Receive ymodem file.
    - Return value: Whether it is successful.
    - Example:

        ```python
        result = filetransfer.ReceiveYmodem()
        ```

- `FileTransfer.SendYmodem(path: str) -> int`：Send ymodem file.
    - Parameter:
        - `path`: File path.
    - Return value: Whether it is successful.
    - Example:

        ```python
        result = filetransfer.SendYmodem("C:\\example.txt")
        ```

- `FileTransfer.SendZmodem() -> int`：Send zmodem file.
    - Return value: Whether it is successful.
    - Example:

        ```python
        result = filetransfer.SendZmodem()
        ```

#### Attribute

- `FileTransfer.DownloadFolder`: Download folder.

### CommandWindow

`CommandWindow` is used to operate the command window, global singleton, called through the crt object, for example:

```python
commandwindow = crt.CommandWindow
```

It includes the following parts:

#### Method

- `CommandWindow.Send()`：Send text to the command window.
    - Example:

        ```python
        commandwindow.Text = 'Hello, quardCRT!'
        commandwindow.Send()
        ```

#### Attribute

- `CommandWindow.SendCharactersImmediately`: Whether to send characters immediately.
- `CommandWindow.SendToAllSessions`: Whether to send to all sessions.
- `CommandWindow.Visible`: Whether the command window is visible.
- `CommandWindow.Text`: Command window text.

### Tab

`Tab` is used to manage tab groups, called through the crt object, for example:

```python
tab = crt.GetActiveTab()   # Get the currently active tab group
```

It includes the following parts:

#### Method

- `Tab.GetScreen(index: int) -> object`：Get the screen of the tab group.
    - Parameter:
        - `index`: Screen index.
    - Return value: Screen object.
    - Example:

        ```python
        screen = tab.GetScreen(0)
        ```

- `Tab.GetSession(index: int) -> object`：Get the session of the tab group.
    - Parameter:
        - `index`: Session index.
    - Return value: Session object.
    - Example:

        ```python
        session = tab.GetSession(0)
        ```

- `Tab.Activate(index: int)`: Activate the specified session of the tab group.
    - Parameter:
        - `index`: Session index.
    - Example:

        ```python
        tab.Activate(0)
        ```

#### Attribute

- `Tab.Number`: The number of sessions in the tab group. Read-only.
