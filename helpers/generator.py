#!python

import random
import string

# 1G = 1024 * 1024 * 1024 = (63 + 1) * 256 * 256 * 256

N = 63

with open("../resources/hugetext.bin", "w", newline='\n') as output:
    for i in range(256 * 256 * 256):
        output.write(''.join(random.choices(string.ascii_lowercase + string.digits, k=N)) + '\n')
