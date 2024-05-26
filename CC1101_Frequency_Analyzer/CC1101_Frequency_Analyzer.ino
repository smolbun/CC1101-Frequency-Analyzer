#include <Arduino.h>
#include <RadioLib.h>
#include <SPI.h>

#define SCK_PIN 10
#define MISO_PIN 8
#define MOSI_PIN 7
#define SS_PIN 4
#define gdo2 3
#define gdo0 1

CC1101 radio = new Module(SS_PIN, gdo0, RADIOLIB_NC, gdo2);

static const uint32_t subghz_frequency_list[] = {
  /* 300 - 348 */
  300000000,
  303875000,
  304250000,
  310000000,
  315000000,
  318000000,

  /* 387 - 464 */
  390000000,
  418000000,
  433075000,
  433420000,
  433920000,
  434420000,
  434775000,
  438900000,

  /* 779 - 928 */
  868350000,
  915000000,
  925000000
};

typedef struct
{
  uint32_t frequency_coarse;
  int rssi_coarse;
  uint32_t frequency_fine;
  int rssi_fine;
} FrequencyRSSI;

const int rssi_threshold = -90;

void setup() {
  Serial.begin(115200);
  while (!Serial) {};
  radio.begin();
  Serial.println("Frequency Analayzer Started!");
}

void loop() {
  int rssi;
  FrequencyRSSI frequency_rssi = {
    .frequency_coarse = 0, .rssi_coarse = -100, .frequency_fine = 0, .rssi_fine = -100
  };

  // First stage: coarse scan
  // Run through common frequency presets to get the most accurate coarse frequency
  size_t array_size = sizeof(subghz_frequency_list) / sizeof(subghz_frequency_list[0]);
  for (size_t i = 0; i < array_size; i++) {
    uint32_t frequency = subghz_frequency_list[i];
    radio.setRxBandwidth(650);
    radio.setFrequency((float)frequency / 1000000.0);
    radio.receiveDirect();
    delay(3);
    rssi = radio.getRSSI();

    if (frequency_rssi.rssi_coarse < rssi) {
      frequency_rssi.rssi_coarse = rssi;
      frequency_rssi.frequency_coarse = frequency;
    }
  }


  // Second stage: fine scan
  if (frequency_rssi.rssi_coarse > rssi_threshold) {
    // for example -0.3 ... 433.92 ... +0.3 step 20KHz
    for (uint32_t i = frequency_rssi.frequency_coarse - 300000; i < frequency_rssi.frequency_coarse + 300000; i += 20000) {
      uint32_t frequency = i;
      radio.setRxBandwidth(58);
      radio.setFrequency((float)frequency / 1000000.0);
      radio.receiveDirect();
      delay(3);
      rssi = radio.getRSSI();

      if (rssi >= frequency_rssi.rssi_coarse) {
        if (frequency_rssi.rssi_fine < rssi) {
          frequency_rssi.rssi_fine = rssi;
          frequency_rssi.frequency_fine = frequency;
        }
      }
    }
  }

  if (frequency_rssi.frequency_fine != 0) {
    // The frequency out would be approximately ±0.02 of the true frequency
    Serial.printf("1. COARSE      Frequency: %.2f  RSSI: %d\n", (float)frequency_rssi.frequency_coarse / 1000000.0, frequency_rssi.rssi_coarse);
    Serial.printf("2. FINE        Frequency: %.2f  RSSI: %d\n\n", (float)frequency_rssi.frequency_fine / 1000000.0, frequency_rssi.rssi_fine);
  }
}
