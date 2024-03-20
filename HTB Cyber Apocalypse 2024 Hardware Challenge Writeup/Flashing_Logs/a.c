#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    uint32_t timestamp;   // Timestamp of the event
    uint8_t eventType;    // Numeric code for type of event // 0 to 255 (0xFF)
    uint16_t userId;      // Numeric user identifier // 0 t0 65535 (0xFFFF)
    uint8_t method;       // Numeric code for unlock method
    uint8_t status;       // Numeric code for status (success, failure)
} SmartLockEvent;

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

int main(){
	uint8_t dat1[] = {35, 197, 21, 102, 214, 0, 160, 3, 1, 1, 0, 0, 207, 91, 108, 133};
	uint8_t dat2[] = {191, 243, 21, 102, 187, 0, 160, 3, 3, 1, 0, 0, 236, 104, 129, 205};
	uint8_t dat3[] = {227, 21, 22, 102, 187, 0, 160, 3, 3, 1, 0, 0, 134, 173, 71, 208};
	uint8_t dat4[] = {239, 127, 22, 102, 214, 0, 160, 3, 3, 1, 0, 0, 209, 155, 216, 44};
	uint32_t crc;
	crc = calculateCRC32(dat1, 12);
	printf("%x\n", crc);
	crc = calculateCRC32(dat2, 12);
	printf("%x\n", crc);
	crc = calculateCRC32(dat3, 12);
	printf("%x\n", crc);
	crc = calculateCRC32(dat4, 12);
	printf("%x\n", crc);
	return 0;
}
