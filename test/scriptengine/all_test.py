import sys
from quardCRT import crt

def main():
    crt.Dialog.MessageBox("crt.ActivePrinter : " + crt.ActivePrinter)
    crt.Dialog.MessageBox("crt.ScriptFullName : " + crt.ScriptFullName)
    crt.Dialog.MessageBox("crt.Version : " + crt.Version)
    crt.Dialog.MessageBox("crt.Screen.CurrentColumn : " + str(crt.Screen.CurrentColumn))
    crt.Dialog.MessageBox("crt.Screen.CurrentRow : " +  str(crt.Screen.CurrentRow))

if __name__ == '__main__':
    main()
