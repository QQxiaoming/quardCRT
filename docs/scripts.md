<div style="text-align: right"><a href="../../en/latest/scripts.html">🇺🇸 English</a> | <a href="../../zh-cn/latest/scripts.html">🇨🇳 简体中文</a> | <a href="../../zh-tw/latest/scripts.html">🇭🇰 繁體中文</a> | <a href="../../ja/latest/scripts.html">🇯🇵 日本語</a></div>

# Scripts

quardCRT supports scripting from version V0.5.0. Scripts are loaded in the form of Python language, and can call quardCRT's API to achieve automation.

## Load scripts

Open the main interface, click the `Scripts` in the menu bar, select `Run...`, select the script file, and click `Open` to load the script.

## Example

Here is a simple script example that displays the version information of quardCRT.

```python
import sys
from quardCRT import crt

def main():
    # Display quardCRT's version
    crt.Dialog.MessageBox("quardCRT version is: " + crt.Version)

if __name__ == '__main__':
    main()
```

quardCRT loaded scripts are no different from regular Python scripts. Now we explain this script line by line.

- `import sys`: Import the `sys` module to get command line arguments.
- `from quardCRT import crt`: Import the API of quardCRT.
- `def main():`: Define a `main` function to execute the main logic of the script.
- `# Display quardCRT's version`: Comment, used to explain the effect of the next line of code.
- `crt.Dialog.MessageBox("quardCRT version is: " + crt.Version)`: Call the API of quardCRT to display a message box, showing the version information of quardCRT.
- `if __name__ == '__main__':`: Determine whether the script is running as the main program.
- `main()`: Call the `main` function to execute the main logic of the script.

## API

The API of quardCRT includes the following parts:

- `crt`: The main API of quardCRT.
- `crt.Dialog`: Used to display dialog boxes.
- `crt.Session`: Used to manage sessions.
- `crt.Screen`: Used to manage screens.

### crt

`crt` is the main API of quardCRT, including the following parts:

- `crt.Version`: The version information of quardCRT.

### crt.Dialog

`crt.Dialog` is used to display dialog boxes, including the following methods:

- `crt.Dialog.MessageBox(message: str, title: str = None, type: int = 0) -> int`: Display a message box.
- `crt.Dialog.InputBox(prompt: str, title: str = None, default: str = None, type: int = 0) -> str`: Display an input box.

### crt.Session

`crt.Session` is used to manage sessions, including the following methods:

- `crt.Session.Connect(hostname: str, port: int, protocol: int, username: str, password: str) -> bool`: Connect to a host.
- `crt.Session.Disconnect()`: Disconnect the current session.

### crt.Screen

`crt.Screen` is used to manage screens, including the following methods:

- `crt.Screen.Send(text: str)`: Send text to the screen.
- `crt.Screen.WaitForString(text: str, timeout: int) -> int`: Wait for the specified text to appear on the screen.
