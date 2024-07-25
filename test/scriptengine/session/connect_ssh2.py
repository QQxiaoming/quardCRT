# Connect to an SSH server using the SSH2 protocol. Specify the
# username and password and hostname on the command line as well as
# some SSH2 protocol specific options.

import sys
from quardCRT import crt

host = "ssh.somecompany.com"
port = 22
user = "myusername"

def main():
    passwd = crt.Dialog.Prompt("Enter password for " + host, "Login", "", True)
    cmd = "-ssh2 %s %s %s %s" % (host, port, user, passwd)
    crt.Session.Connect(cmd)

if __name__ == '__main__':
    main()
