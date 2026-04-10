from quardCRT import crt


def main():
    host = crt.Dialog.Prompt("Telnet host:", "Telnet Login", "", False)
    if not host:
        return

    port_text = crt.Dialog.Prompt("Telnet port:", "Telnet Login", "23", False)
    if not port_text:
        return

    username = crt.Dialog.Prompt("Username:", "Telnet Login", "", False)
    if not username:
        return

    password = crt.Dialog.Prompt("Password:", "Telnet Login", "", True)
    if password == "":
        return

    login_prompt = crt.Dialog.Prompt("Login prompt:", "Telnet Login", "login:", False)
    if not login_prompt:
        return

    password_prompt = crt.Dialog.Prompt("Password prompt:", "Telnet Login", "Password:", False)
    if not password_prompt:
        return

    shell_prompt = crt.Dialog.Prompt("Shell prompt:", "Telnet Login", "$ ", False)
    if not shell_prompt:
        return

    timeout_text = crt.Dialog.Prompt("Timeout in milliseconds:", "Telnet Login", "5000", False)
    if not timeout_text:
        return

    timeout = int(timeout_text)
    crt.Screen.Synchronous = True
    try:
        result = crt.Session.Connect("-telnet %s %s" % (host, port_text))
        if result != 0:
            crt.Dialog.MessageBox(
                "Connect failed.\n\nLast error: " + crt.GetLastErrorMessage(),
                "Telnet Login"
            )
            return

        if not crt.Screen.WaitForString(login_prompt, timeout, False):
            crt.Dialog.MessageBox("Timed out waiting for login prompt.", "Telnet Login")
            return

        crt.Screen.Send(username + "\r")

        if not crt.Screen.WaitForString(password_prompt, timeout, False):
            crt.Dialog.MessageBox("Timed out waiting for password prompt.", "Telnet Login")
            return

        crt.Screen.Send(password + "\r")

        if crt.Screen.WaitForString(shell_prompt, timeout, False):
            crt.Dialog.MessageBox("Login sequence completed.", "Telnet Login")
        else:
            crt.Dialog.MessageBox(
                "Login was sent, but the shell prompt was not matched.\n"
                "Adjust shell_prompt or timeout in the script.",
                "Telnet Login"
            )
    finally:
        crt.Screen.Synchronous = False


if __name__ == '__main__':
    main()