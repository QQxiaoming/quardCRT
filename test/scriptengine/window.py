import sys
from quardCRT import crt

def main():
    crt.Sleep(3000)
    crt.Window.Show(0) # hide
    crt.Sleep(3000)
    crt.Window.Show(1) # show
    crt.Sleep(3000)
    crt.Window.Show(2) # minimize
    crt.Sleep(3000)
    crt.Window.Activate() # restore
    crt.Sleep(3000)
    crt.Window.Show(3) # maximize

if __name__ == '__main__':
    main()
