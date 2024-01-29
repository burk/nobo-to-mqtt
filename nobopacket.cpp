#include <memory>
#include "nobopacket.h"
#include "util.h"

boolean is_valid(uint8_t *data)
{
    uint16_t calculated = crc16(data, LENGTH - 2);
    uint16_t from_message = ((uint16_t)data[LENGTH - 2]) * 256 + (uint16_t)data[LENGTH - 1];
    return calculated == from_message;
}