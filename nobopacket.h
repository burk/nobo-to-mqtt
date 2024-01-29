#ifndef NOBOPACKET_H
#define NOBOPACKET_H

#include <Arduino.h>
#include <memory>
#include "util.h"

#define LENGTH 32 + 2 + 4

enum PacketType {
  UNKNOWN_PACKET,
  STATUS_PACKET
};

class Packet {
  public:
    uint8_t data[LENGTH];

    uint32_t address(int start) {
      uint32_t result = 0;
      for (int i = 0; i < 4; ++i) {
        result = result * 256 + (uint32_t)data[i + start];
      }
      return result;
    }

    uint32_t address_reversed(int start) {
      uint32_t result = 0;
      for (int i = 3; i >= 0; --i) {
        result = result * 256 + (uint32_t)data[i + start];
      }
      return result;
    }

    Packet(uint8_t *data) {
      for (int i = 0; i < LENGTH; ++i) {
        this->data[i] = data[i];
      }
    }

    uint32_t to() {
      return address(0);
    }

    uint32_t addr1() {
      return address_reversed(7);
    }

    uint32_t addr2() {
      return address_reversed(11);
    }

    uint32_t addr3() {
      return address_reversed(15);
    }

    uint32_t from() {
      uint32_t addr = addr3();
      if (addr == 0) {
        return addr2();
      } else {
        return addr;
      }
    }

    float temperature() {
      return (float) (data[28] * 256 + data[27]) / 128.0;
    }

    uint8_t seconds_on() {
      return data[30];
    }

    float low_temp() {
      return ((float)(255 - data[25])) / 8.0;
    }

    float high_temp() {
      return ((float)(255 - data[26])) / 8.0;
    }

    bool is_status() {
      return is_hub(to()) && temperature() > 0.0 && temperature() < 50.0;
    }

    bool is_setting() {
      if (is_hub(to())) {
        return false;
      }
      float lo = low_temp();
      float hi = high_temp();
      return 5.0 < lo && lo < 40.0 && 5.0 < hi && hi < 40.0;
    }

    String to_json() {
      if (is_status()) {
        return status_json();
      } else if (is_setting()) {
        return setting_json();
      }
    }

    String status_json() {
      String result = "{";
      result += "\"to\": \"" + format_address(to()) + "\", ";
      result += "\"from\": \"" + format_address(from()) + "\", ";
      result += "\"temperature\": " + String(temperature()) + ", ";
      result += "\"seconds_on\": " + String(seconds_on());
      result += "}";
      return result;
    }

    String setting_json() {
      String result = "{";
      result += "\"to\": \"" + format_address(to()) + "\", ";
      result += "\"from\": \"" + format_address(from()) + "\", ";
      result += "\"low\": " + String(low_temp()) + ", ";
      result += "\"high\": " + String(high_temp());
      result += "}";
      return result;
    }
};

boolean is_valid(uint8_t *data);

#endif
