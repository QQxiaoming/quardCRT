import sys
from quardCRT import crt

def main():
    activeTab = crt.GetActiveTab()
    if activeTab.Number == 0:
        crt.Dialog.MessageBox('ActiveTab has no session.')
    else :
        screen = activeTab.GetScreen(0)
        crt.Dialog.MessageBox("Screen's global ID is " + str(screen.Id) + ".")

if __name__ == '__main__':
    main()
