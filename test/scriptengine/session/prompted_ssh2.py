from quardCRT import crt


def main():
    host = crt.Dialog.Prompt("SSH host:", "SSH2 Connect", "", False)
    if not host:
        return

    port_text = crt.Dialog.Prompt("SSH port:", "SSH2 Connect", "22", False)
    if not port_text:
        return

    user = crt.Dialog.Prompt("Username:", "SSH2 Connect", "root", False)
    if not user:
        return

    password = crt.Dialog.Prompt("Password:", "SSH2 Connect", "", True)
    if password == "":
        return

    cmd = "-ssh2 %s %s %s %s" % (host, port_text, user, password)
    result = crt.Session.Connect(cmd)
    if result != 0:
        crt.Dialog.MessageBox(
            "Connect failed.\n\nLast error: " + crt.GetLastErrorMessage(),
            "SSH2 Connect"
        )


if __name__ == '__main__':
    main()