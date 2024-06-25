import sys
from quardCRT import crt

def main():
    crt.Dialog.MessageBox(str(crt.Arguments.Count))
    crt.Dialog.MessageBox("crt.Arguments[0]: " + crt.Arguments[0])

if __name__ == '__main__':
    main()
