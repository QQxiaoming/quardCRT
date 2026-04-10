from datetime import datetime

from quardCRT import crt


def main():
    if not crt.Session.Connected:
        crt.Dialog.MessageBox("No active connected session.", "Repeat Command Logger")
        return

    command = crt.Dialog.Prompt("Command to send:", "Repeat Command Logger", "help", False)
    if not command:
        return

    repeat_text = crt.Dialog.Prompt("Repeat count:", "Repeat Command Logger", "5", False)
    if not repeat_text:
        return

    interval_text = crt.Dialog.Prompt("Interval in milliseconds:", "Repeat Command Logger", "1000", False)
    if not interval_text:
        return

    prompt_text = crt.Dialog.Prompt("Expected prompt:", "Repeat Command Logger", "$ ", False)
    if not prompt_text:
        return

    log_path = crt.Dialog.FileSaveDialog(
        "Save command log",
        "Save",
        "command_log.txt",
        "Text Files (*.txt)|*.txt|All Files (*.*)|*.*"
    )
    if not log_path:
        return

    repeat_count = int(repeat_text)
    interval_ms = int(interval_text)
    crt.Screen.Synchronous = True
    try:
        with open(log_path, "w", encoding="utf-8") as handle:
            for index in range(repeat_count):
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                crt.Screen.Send(command + "\r")
                output = crt.Screen.ReadString([prompt_text], 5000, False)

                handle.write("=== iteration %d @ %s ===\n" % (index + 1, timestamp))
                if output:
                    handle.write(output)
                    if not output.endswith("\n"):
                        handle.write("\n")
                else:
                    handle.write("Timed out waiting for prompt: %s\n" % prompt_text)
                handle.write("\n")

                if index + 1 < repeat_count:
                    crt.Sleep(interval_ms)
    finally:
        crt.Screen.Synchronous = False

    crt.Dialog.MessageBox("Saved command log to:\n" + log_path, "Repeat Command Logger")


if __name__ == '__main__':
    main()