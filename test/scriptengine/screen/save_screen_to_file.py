from quardCRT import crt


def main():
    default_name = "screen_dump.txt"
    path = crt.Dialog.FileSaveDialog(
        "Save current screen text",
        "Save",
        default_name,
        "Text Files (*.txt)|*.txt|All Files (*.*)|*.*"
    )
    if not path:
        return

    content = crt.Screen.Get2(1, 1, crt.Screen.Rows, crt.Screen.Columns)
    with open(path, "w", encoding="utf-8") as handle:
        handle.write(content)

    crt.Dialog.MessageBox("Saved screen text to:\n" + path, "Save Screen")


if __name__ == '__main__':
    main()