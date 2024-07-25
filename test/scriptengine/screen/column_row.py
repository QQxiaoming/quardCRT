import sys
from quardCRT import crt

def main():
    crt.Dialog.MessageBox(
        "crt.Screen.CurrentColumn : " + str(crt.Screen.CurrentColumn) + "\n" +
        "crt.Screen.CurrentRow : " + str(crt.Screen.CurrentRow) + "\n" +
        "crt.Screen.Columns : " + str(crt.Screen.Columns) + "\n" +
        "crt.Screen.Rows : " + str(crt.Screen.Rows)
    )

if __name__ == '__main__':
    main()
