# Connect to a telnet server and automate the initial login sequence.
# Note that synchronous mode is enabled to prevent server output from
# potentially being missed.

import sys
from quardCRT import crt

def main():
    crt.Screen.Synchronous = True

    crt.Session.Connect("-telnet login.myhost.com 23")

    crt.Screen.WaitForString("ogin:")
    crt.Screen.Send("myusername\r")
    crt.Screen.WaitForString("assword:")
    crt.Screen.Send("mypassword\r")

    crt.Screen.Synchronous = False

if __name__ == '__main__':
    main()