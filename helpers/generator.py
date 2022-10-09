#!python

import random
import string

# 2G = 2 * 1024 * 1024 * 1024
# 2G = 128 * 256 * 256 * 256
# 2G = 1/2 * 65536 * 65536

N = 128

with open("test.bin", "w") as output:
    for i in range(256):
        for j in range(256):
            for k in range(256):
                output.write(''.join(random.choices(string.ascii_lowercase + string.digits, k=N)) + '\n')
