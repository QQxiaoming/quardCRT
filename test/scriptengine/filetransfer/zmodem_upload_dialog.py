from quardCRT import crt


def main():
    path = crt.Dialog.FileOpenDialog(
        "Select file to upload with Zmodem",
        "Open",
        "",
        "All Files (*.*)|*.*"
    )
    if not path:
        return

    crt.FileTransfer.ClearUploadList()
    crt.FileTransfer.AddToUploadList(path)

    result = crt.FileTransfer.SendZmodem()
    if result == 0:
        crt.Dialog.MessageBox("Zmodem upload started for:\n" + path, "Zmodem Upload")
    else:
        crt.Dialog.MessageBox(
            "Failed to start Zmodem upload.\n\nLast error: " + crt.GetLastErrorMessage(),
            "Zmodem Upload"
        )


if __name__ == '__main__':
    main()