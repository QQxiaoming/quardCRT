import sys
from quardCRT import crt

def main():
    try:
        tab = crt.GetActiveTab()
    except ScriptError:
        errmsg = crt.GetLastErrorMessage()
        crt.Dialog.MessageBox("Error Code: " + errmsg)

if __name__ == '__main__':
    main()
