from quardCRT import crt


def main():
    if not crt.Session.Connected:
        crt.Dialog.MessageBox("No active connected session.", "Send Command")
        return

    command = crt.Dialog.Prompt("Command to send:", "Send Command", "help", False)
    if not command:
        return

    prompts_text = crt.Dialog.Prompt(
        "Prompts to wait for, separated by | :",
        "Send Command",
        "# |$ |>",
        False
    )
    if not prompts_text:
        return

    timeout_text = crt.Dialog.Prompt(
        "Timeout in milliseconds:",
        "Send Command",
        "5000",
        False
    )
    if not timeout_text:
        return

    ignore_case_text = crt.Dialog.Prompt(
        "Ignore case? Enter true or false:",
        "Send Command",
        "false",
        False
    )
    if not ignore_case_text:
        return

    prompt_list = [prompt.strip() for prompt in prompts_text.split("|") if prompt.strip()]
    if not prompt_list:
        crt.Dialog.MessageBox("At least one prompt is required.", "Send Command")
        return

    try:
        timeout = int(timeout_text)
    except ValueError:
        crt.Dialog.MessageBox("Timeout must be an integer.", "Send Command")
        return

    ignore_case = ignore_case_text.strip().lower() in ("1", "true", "yes", "y", "on")

    crt.Screen.Synchronous = True
    try:
        crt.Screen.Send(command + "\r")
        output = crt.Screen.ReadString(prompt_list, timeout, ignore_case)
    finally:
        crt.Screen.Synchronous = False

    if output:
        crt.Dialog.MessageBox(output, "Command Output")
    else:
        crt.Dialog.MessageBox(
            "Timed out waiting for a prompt. Adjust prompts or timeout and try again.",
            "Send Command"
        )


if __name__ == '__main__':
    main()