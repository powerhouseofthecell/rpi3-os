def getVA(l1_idx, l2_idx, l3_idx, p_off):
    base = '1' * 25
    l1_idx = bin(l1_idx).split('0b')[1].zfill(9)
    l2_idx = bin(l2_idx).split('0b')[1].zfill(9)
    l3_idx = bin(l3_idx).split('0b')[1].zfill(9)
    p_off = bin(p_off).split('0b')[1].zfill(12)
    return hex(int(base + l1_idx + l2_idx + l3_idx + p_off, 2))

import sys

if __name__ == '__main__':
    print(getVA(
        int(sys.argv[1]),
        int(sys.argv[2]),
        int(sys.argv[3]),
        int(sys.argv[4], 16)
    ))