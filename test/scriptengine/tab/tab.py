import sys
from quardCRT import crt

def main():
    activeTab = crt.GetActiveTab()
    crt.Dialog.MessageBox('ActiveTab has ' + str(activeTab.Number) + 'session(s).')

if __name__ == '__main__':
    main()
