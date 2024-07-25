import sys
from quardCRT import crt

def main():
    crt.Dialog.MessageBox("Clipboard: " + crt.Clipboard.Text)

if __name__ == '__main__':
    main()
