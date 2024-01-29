#include <Arduino.h>
#include "util.h"
#include "config.h"

uint16_t crc16(uint8_t* pData, int length)
{
  // From: https://gist.github.com/tijnkooijmans/10981093
    uint8_t i;
    uint16_t wCrc = 0xffff;
    while (length--) {
        wCrc ^= *(uint8_t *)pData++ << 8;
        for (i=0; i < 8; i++)
            wCrc = wCrc & 0x8000 ? (wCrc << 1) ^ 0x1021 : wCrc << 1;
    }
    return wCrc & 0xffff;
}

String format_address(uint32_t address)
{
  switch (address) {
    case SOVEROM:
      return "Soverom";
    case KJOKKEN:
      return "Kjokken";
    case STUE:
      return "Stue";
    case BAD:
      return "Bad";
    case HUB:
      return "Hub";
    case 0x0:
      return "0";
    default: 
      return String(address, HEX);
  }
}

bool is_hub(uint32_t address)
{
  return address == HUB;
}
