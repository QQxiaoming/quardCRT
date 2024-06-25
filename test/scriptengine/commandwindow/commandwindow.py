import sys
from quardCRT import crt

def main():
    crt.CommandWindow.Text = 'Hello, quardCRT!'
    crt.CommandWindow.Send()

if __name__ == '__main__':
    main()
