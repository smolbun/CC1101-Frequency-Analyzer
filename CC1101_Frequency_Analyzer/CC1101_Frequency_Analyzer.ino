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
  300000000, 302757000, 303875000, 303900000, 304250000, 307000000, 307500000, 307800000,
  309000000, 310000000, 312000000, 312100000, 313000000, 313850000, 314000000, 314350000,
  314980000, 315000000, 318000000, 330000000, 345000000, 348000000, 350000000, 387000000,
  390000000, 418000000, 430000000, 431000000, 431500000, 433075000, 433220000, 433420000,
  433657070, 433889000, 433920000, 434075000, 434176948, 434390000, 434420000, 434775000,
  438900000, 440175000, 464000000, 779000000, 868350000, 868400000, 868800000, 868950000,
  906400000, 915000000, 925000000, 928000000
};

typedef struct
{
  uint32_t frequency_coarse;
  int rssi_coarse;
  uint32_t frequency_fine;
  int rssi_fine;
} FrequencyRSSI;

const int rssi_threshold = -78;

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
  radio.setRxBandwidth(650);
  size_t array_size = sizeof(subghz_frequency_list) / sizeof(subghz_frequency_list[0]);
  for (size_t i = 0; i < array_size; i++) {
    uint32_t frequency = subghz_frequency_list[i];
    if (frequency != 467750000 && frequency != 464000000 && frequency != 390000000 && frequency != 312000000 && frequency != 312100000 && frequency != 312200000 && frequency != 440175000) {
      radio.setFrequency((float)frequency / 1000000.0);
      radio.receiveDirect();
      delay(2);
      rssi = radio.getRSSI();

      if (frequency_rssi.rssi_coarse < rssi) {
        frequency_rssi.rssi_coarse = rssi;
        frequency_rssi.frequency_coarse = frequency;
      }
    }
  }

  // Second stage: fine scan
  if (frequency_rssi.rssi_coarse > rssi_threshold) {
    // for example -0.3 ... 433.92 ... +0.3 step 20KHz
    radio.setRxBandwidth(58);
    for (uint32_t i = frequency_rssi.frequency_coarse - 300000; i < frequency_rssi.frequency_coarse + 300000; i += 20000) {
      uint32_t frequency = i;
      radio.setFrequency((float)frequency / 1000000.0);
      radio.receiveDirect();
      delay(2);
      rssi = radio.getRSSI();

      if (frequency_rssi.rssi_fine < rssi) {
        frequency_rssi.rssi_fine = rssi;
        frequency_rssi.frequency_fine = frequency;
      }
    }
  }

  // Deliver results fine
  if (frequency_rssi.rssi_fine > rssi_threshold) {
    Serial.printf("FINE        Frequency: %.2f  RSSI: %d\n", (float)frequency_rssi.frequency_fine / 1000000.0, frequency_rssi.rssi_fine);
  }

  // Deliver results coarse
  else if (frequency_rssi.rssi_coarse > rssi_threshold) {
    Serial.printf("COARSE      Frequency: %.2f  RSSI: %d\n", (float)frequency_rssi.frequency_coarse / 1000000.0, frequency_rssi.rssi_coarse);
  }
}
