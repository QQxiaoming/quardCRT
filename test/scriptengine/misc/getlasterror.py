import sys
from quardCRT import crt

def main():
    try:
        tab = crt.GetActiveTab()
    except ScriptError:
        errcode = crt.GetLastError()
        crt.Dialog.MessageBox("Error Code: " + errcode.message)

if __name__ == '__main__':
    main()
