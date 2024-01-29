#include <WiFi.h>
#include <PubSubClient.h>

#include <RadioLib.h>
#include <memory>
#include "nobopacket.h"
#include "util.h"
#include "board.h"
#include "config.h"

#include "freertos/ringbuf.h"


SX1276 radio = new Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);
WiFiClient espClient;
PubSubClient client(espClient);

RingbufHandle_t buf_handle;
TaskHandle_t rf_task;

void setup() {
  
    delay(1500);
    uint8_t syncWord[] = {0x55, 0x99};
    
    pinMode(RADIO_TXCO_ENABLE, OUTPUT);
    digitalWrite(RADIO_TXCO_ENABLE, HIGH);

    Serial.begin(115200);
    SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("WiFI connected!");

    Serial.print(F("[SX1276] Initializing ... "));
    int state = radio.beginFSK(
      868.4,  // Frequency,
      100.0,  // Bit rate (kbps).
      50.0,   // Frequency deviation.
      100.0,  // Receiver bandwidth.
      10,     // Output power (dbm).
      8,      // Preamble length in bits.
      false); // Enable OOK (no thanks).

    if (state == RADIOLIB_ERR_NONE) {
      Serial.println(F("success!"));
      radio.packetMode();
      radio.setGain(0); // Automatic gain control.
      radio.setSyncWord(syncWord, 2);
      radio.setDataShaping(RADIOLIB_SHAPING_0_5);
      radio.setEncoding(RADIOLIB_ENCODING_MANCHESTER);
      radio.setCrcFiltering(false);
      radio.fixedPacketLengthMode(LENGTH);
      radio.setCRC(false);
      radio.setCurrentLimit(120);
    } else {
      Serial.print(F("failed, code "));
      Serial.println(state);
      while (true);
    }

    client.setServer(mqtt_broker, 1883);
    while (!client.connected()) {
      String client_id = "nobo-to-mqtt-";
      client_id += String(WiFi.macAddress());
      Serial.printf("Client %s attempting MQTT connection...\n", client_id.c_str());
      if (client.connect(client_id.c_str())) {
        Serial.println("connected");
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" trying again in 5 seconds ");
        delay(5000);
      }
    }

    buf_handle = xRingbufferCreate(1028, RINGBUF_TYPE_NOSPLIT);
  if (buf_handle == NULL) {
      Serial.println("Failed to create ring buffer");
      while (true);
  }

  xTaskCreatePinnedToCore(
      rf_task_function, /* Function to implement the task */
      "rf_task",        /* Name of the task */
      10000,            /* Stack size in words */
      NULL,             /* Task input parameter */
      0,                /* Priority of the task */
      &rf_task,         /* Task handle. */
      0);               /* Core where the task should run */
}


void rf_task_function(void *parameters) {
  for (;;) {

  }
}

void loop() {
  uint8_t data[LENGTH];
  int16_t status = radio.receive(data, LENGTH);

  if (status == RADIOLIB_ERR_NONE) {
    for (int i = 0; i < LENGTH; ++i) {
      data[i] = ~data[i];
    }
    if (is_valid(data)) {
      Packet packet = Packet(data);
      Serial.print(F("To: "));
      Serial.print(format_address(packet.to()));
      Serial.print("\tFrom: ");
      Serial.print(format_address(packet.from()));
      Serial.print("\t");
      Serial.print(format_address(packet.addr1()));
      Serial.print("\t");
      Serial.print(format_address(packet.addr2()));
      if (packet.is_status()) {
        Serial.print(F(" temp: "));
        Serial.print(packet.temperature());
        Serial.print(" on: ");
        Serial.print(packet.seconds_on());
      } else if (packet.is_setting()) {
        Serial.print(F(" low: "));
        Serial.print(packet.low_temp());
        Serial.print(F(" high: "));
        Serial.print(packet.high_temp());
      }
      Serial.println("");
      if (packet.is_status()) {
        client.publish(mqtt_topic, packet.to_json().c_str());
      }
    } else {
      //Serial.println("  invalid packet");
    }
    
  }
  client.loop();
}

void p(uint8_t X) {
   if (X < 16) {Serial.print("0");}
   Serial.print(X, HEX);
}
