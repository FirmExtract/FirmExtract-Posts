#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <wiringPiSPI.h>
#include "W25Q128.h" // Our custom chip is compatible with the original W25Q128XX design

#define SPI_CHANNEL 0 // /dev/spidev0.0
//#define SPI_CHANNEL 1 // /dev/spidev0.1

#define CRC_SIZE 4 // Size of the CRC data in bytes
#define KEY_SIZE 12 // Size of the key



// SmartLockEvent structure definition
typedef struct {
    uint32_t timestamp;   // Timestamp of the event
    uint8_t eventType;    // Numeric code for type of event // 0 to 255 (0xFF)
    uint16_t userId;      // Numeric user identifier // 0 t0 65535 (0xFFFF)
    uint8_t method;       // Numeric code for unlock method
    uint8_t status;       // Numeric code for status (success, failure)
} SmartLockEvent;


// Function Prototypes
int log_event(const SmartLockEvent event, uint32_t sector, uint32_t address);
uint32_t calculateCRC32(const uint8_t *data, size_t length);
void write_to_flash(uint32_t sector, uint32_t address, uint8_t *data, size_t length);

// CRC-32 calculation function
uint32_t calculateCRC32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; ++j) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return ~crc;
}


bool verify_flashMemory() {
    uint8_t jedc[3]; 
    uint8_t uid[8];     
    uint8_t buf[256];     
    uint8_t wdata[26];    
    uint8_t i;
  
    uint16_t n;    

    bool jedecid_match = true; // Assume true, prove false
    bool uid_match = true; // Assume true, prove false
          

    // JEDEC ID to verify against 
    uint8_t expectedJedec[3] = {0xEF, 0x40, 0x18};

    // UID to verify against 
    uint8_t expectedUID[8] = {0xd2, 0x66, 0xb4, 0x21, 0x83, 0x1f, 0x09, 0x2b};


    // SPI channel 0 at 2MHz.
    // Start SPI channel 0 with 2MHz
    if (wiringPiSPISetup(SPI_CHANNEL, 2000000) < 0) {
      printf("SPISetup failed:\n");
    }
    

    // Start Flash Memory
    W25Q128_begin(SPI_CHANNEL);
    
    // JEDEC ID Get
    //W25Q128_readManufacturer(buf);
    W25Q128_readManufacturer(jedc);
    printf("JEDEC ID : ");
    for (i=0; i< 3; i++) {
      printf("%x ",jedc[i]);
    }
    
    // Iterate over the array and compare elements
    for (int i = 0; i < sizeof(jedc)/sizeof(jedc[0]); ++i) {
        if (jedc[i] != expectedJedec[i]) {
            jedecid_match = false; // Set match to false if any element doesn't match
            break; // No need to check further if a mismatch is found
        }
    }

    if (jedecid_match) {
        printf("JEDEC ID verified successfully.\n");
    } else {
        printf("JEDEC ID does not match.\n");
        return 0;
    }

    // Unique ID
    // Unique ID Get
    W25Q128_readUniqieID(uid);
    printf("Unique ID : ");
    for (i=0; i< 8; i++) {
      printf("%x ",uid[i]);
    }
    printf("\n");
  
    // Iterate over the array and compare elements
    for (int i = 0; i < sizeof(uid)/sizeof(uid[0]); ++i) {
        if (uid[i] != expectedUID[i]) {
            uid_match = false; // Set match to false if any element doesn't match
            break; // No need to check further if a mismatch is found
        }
    }

    if (uid_match) {
        printf("UID verified successfully.\n");
    } else {
        printf("UID does not match.\n");
        return 0;
    }

    return 1;
}
// verify_flashmemory()는 UID랑 JEDEC ID 확인하는 함수.

// Implementations
int log_event(const SmartLockEvent event, uint32_t sector, uint32_t address) {

    bool memory_verified = false;
    uint8_t i;
    uint16_t n;  
    uint8_t buf[256];   


    memory_verified = verify_flashMemory();
    if (!memory_verified) return 0;

     // Start Flash Memory
    W25Q128_begin(SPI_CHANNEL);
    

    // Erase data by Sector
    if (address == 0){
        printf("ERASE SECTOR!");
        n = W25Q128_eraseSector(0, true);
        printf("Erase Sector(0): n=%d\n",n);
        memset(buf,0,256);
        n =  W25Q128_read (0, buf, 256);
       
    }

    uint8_t buffer[sizeof(SmartLockEvent) + sizeof(uint32_t)]; // Buffer for event and CRC
    uint32_t crc;

    memset(buffer, 0, sizeof(SmartLockEvent) + sizeof(uint32_t));

    // Serialize the event
    memcpy(buffer, &event, sizeof(SmartLockEvent));

    // Calculate CRC for the serialized event
    crc = calculateCRC32(buffer, sizeof(SmartLockEvent));

    // Append CRC to the buffer
    memcpy(buffer + sizeof(SmartLockEvent), &crc, sizeof(crc));

//buffer = SmarLockEvent 넣고 뒤에 4바이트 crc

    // Print the SmartLockEvent for debugging
    printf("SmartLockEvent:\n");
    printf("Timestamp: %u\n", event.timestamp);
    printf("EventType: %u\n", event.eventType);
    printf("UserId: %u\n", event.userId);
    printf("Method: %u\n", event.method);
    printf("Status: %u\n", event.status);

    // Print the serialized buffer (including CRC) for debugging
    printf("Serialized Buffer (including CRC):");
    for (size_t i = 0; i < sizeof(buffer); ++i) {
        if (i % 16 == 0) printf("\n"); // New line for readability every 16 bytes
        printf("%02X ", buffer[i]);
    }
    printf("\n");


    // Write the buffer to flash
    write_to_flash(sector, address, buffer, sizeof(buffer));


    // Read 256 byte data from Address=0
    memset(buf,0,256);
    n =  W25Q128_read(0, buf, 256);
    printf("Read Data: n=%d\n",n);
    dump(buf,256);

    return 1;
}


// encrypts log events 
void encrypt_data(uint8_t *data, size_t data_length, uint8_t register_number, uint32_t address) {
    uint8_t key[KEY_SIZE];

    read_security_register(register_number, 0x52, key); // register, address
    
    printf("Data before encryption (including CRC):\n");
    for(size_t i = 0; i < data_length; ++i) {
        printf("%02X ", data[i]);
    }
    printf("\n");

    // Print the CRC32 checksum before encryption (assuming the original data includes CRC)
    uint32_t crc_before_encryption = calculateCRC32(data, data_length - CRC_SIZE);
    printf("CRC32 before encryption: 0x%08X\n", crc_before_encryption);

    // Apply encryption to data, excluding CRC, using the key
    for (size_t i = 0; i < data_length - CRC_SIZE; ++i) { // Exclude CRC data from encryption
        data[i] ^= key[i % KEY_SIZE]; // Cycle through  key bytes
    }

    printf("Data after encryption (including CRC):\n");
    for(size_t i = 0; i < data_length; ++i) {
        printf("%02X ", data[i]);
    }
    printf("\n");


}

void write_to_flash(uint32_t sector, uint32_t address, uint8_t *data, size_t length) {
    printf("Writing to flash at sector %u, address %u\n", sector, address);
    
    uint8_t i;
    uint16_t n;  

    encrypt_data(data, length, 1, address);  

    n =  W25Q128_pageWrite(sector, address, data, 16);
    printf("page_write(0,10,d,26): n=%d\n",n);

}
