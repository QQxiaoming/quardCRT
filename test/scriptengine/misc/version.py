import sys
from quardCRT import crt

def main():
    # Display quardCRT's version
    crt.Dialog.MessageBox("quardCRT version is: " + crt.Version)

if __name__ == '__main__':
    main()
