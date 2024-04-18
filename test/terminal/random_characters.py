# Randomly generate Unicode character output,
# 8 per line, one line generated every 500ms

import random
import time

def main():
    while True:
        print(''.join([chr(random.choice(list(range(0x0000, 0xD800)) + list(range(0xE000, 0xFFFF)))) for _ in range(8)])
                + '\n', end='')
        time.sleep(0.5)

if __name__ == '__main__':
    main()
