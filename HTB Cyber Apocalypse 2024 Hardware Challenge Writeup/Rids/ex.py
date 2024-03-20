from pwn import *
import struct
import json

server = "nc 94.237.50.51 51179"
server = server.split()

def exchange(hex_list, value=0):

    cs = 0
    usb_device_url = 'ftdi://ftdi:2232h/1'

    # Convert hex list to strings and prepare the command data
    command_data = {
        "tool": "pyftdi",
        "cs_pin":  cs,
        "url":  usb_device_url,
        "data_out": [hex(x) for x in hex_list],  # Convert hex numbers to hex strings
        "readlen": value
    }
    
    with remote(server[1], server[2]) as s:
        
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
data = exchange([0x03, 0x00, 0x00, 0x00], 10200)
result = ''.join(chr(num) for num in data if num != 255)
print(result)
