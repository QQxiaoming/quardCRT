from quardCRT import crt


def main():
    active_tab = crt.GetActiveTab()
    if active_tab.Number == 0:
        crt.Dialog.MessageBox("Active tab has no session.", "Send To All Sessions")
        return

    command = crt.Dialog.Prompt(
        "Command to send to every session in the active tab:",
        "Send To All Sessions",
        "",
        False
    )
    if not command:
        return

    for index in range(active_tab.Number):
        screen = active_tab.GetScreen(index)
        screen.Send(command + "\r")

    crt.Dialog.MessageBox(
        "Sent command to %d session(s)." % active_tab.Number,
        "Send To All Sessions"
    )


if __name__ == '__main__':
    main()