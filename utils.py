# prints each bit starting at least significant (assuming big-endian)
def print_bits(n):
    b = bin(n).split('0b')[1][::-1]

    for i, c in enumerate(b):
        print(i, c)

# returns the number from setting each bit in the list
def set_bits(lst):
    num = 0
    for bit in lst:
        num |= (1<<bit)
    return num
