#ifndef UTIL_H
#define UTIL_H

uint16_t crc16(uint8_t* pData, int length);
String format_address(uint32_t address);
bool is_hub(uint32_t address);

#endif
