from pwn import *

def read_byte(address):
    global flag
    addr_binary = "{0:b}".format(address)
    addr_str = str(addr_binary).rjust(11,"0").replace("1","5")
    addr_list = [int(x) for x in list(addr_str)]
    addr_list[1] = 12 # For Only purpose for FLAG Because by setting to 12V, we can read Chip ID
    comm = "set_address_pins(" + str(addr_list) + ")"
    print(comm)
    r.sendlineafter("> ", comm)
    r.sendlineafter("> ", "read_byte()")
    r.recvuntil("Read ")
    flag += chr(int(r.recvuntil(" ")[:-1],16))
    print(r.recvuntil("\n")[:-1])
    print(flag)

server = "nc 83.136.251.7 49687"
server = server.split()

r = remote(server[1], server[2])

r.sendlineafter("> ", "set_ce_pin(0.5)")
r.sendlineafter("> ", "set_oe_pin(0.5)")
r.sendlineafter("> ", "set_we_pin(4)")

flag = ""

for i in range(0x7e0, 0x800):
    read_byte(i)

r.interactive()
