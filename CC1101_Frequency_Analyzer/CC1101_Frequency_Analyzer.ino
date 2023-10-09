#include <Arduino.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <SPI.h>

#define SCK_PIN 18
#define MISO_PIN 16
#define MOSI_PIN 19
#define SS_PIN 17

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
    925000000};

typedef struct
{
    uint32_t frequency_coarse;
    int rssi_coarse;
    uint32_t frequency_fine;
    int rssi_fine;
} FrequencyRSSI;

const int rssi_threshold = -70;

void setup()
{
    Serial.begin(115200);

    ELECHOUSE_cc1101.setSpiPin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
#ifdef ARDUINO_ARCH_RP2040
    SPI.setRX(MISO_PIN);
    SPI.setCS(SS_PIN);
    SPI.setSCK(SCK_PIN);
    SPI.setTX(MOSI_PIN);
#endif
    if (!ELECHOUSE_cc1101.getCC1101())
    {
        Serial.println("Connection Error");
    }
    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setRxBW(58);
    // ELECHOUSE_cc1101.setModulation(2);
    delay(1000);
    Serial.println("Frequency Analayzer Started!");
}

void loop()
{
    int rssi;
    FrequencyRSSI frequency_rssi = {
        .frequency_coarse = 0, .rssi_coarse = -100, .frequency_fine = 0, .rssi_fine = -100};

    // First stage: coarse scan
    // Run through common frequency presets to get the most accurate coarse frequency
    size_t array_size = sizeof(subghz_frequency_list) / sizeof(subghz_frequency_list[0]);
    for (size_t i = 0; i < array_size; i++)
    {
        uint32_t frequency = subghz_frequency_list[i];
        ELECHOUSE_cc1101.setMHZ((float)frequency / 1000000.0);
        ELECHOUSE_cc1101.SetRx();
        delayMicroseconds(3500);
        rssi = ELECHOUSE_cc1101.getRssi();

        if (frequency_rssi.rssi_coarse < rssi)
        {
            frequency_rssi.rssi_coarse = rssi;
            frequency_rssi.frequency_coarse = frequency;
        }
    }

    // Second stage: fine scan
    if (frequency_rssi.rssi_coarse > rssi_threshold)
    {
        // for example -0.3 ... 433.92 ... +0.3 step 20KHz
        for (uint32_t i = frequency_rssi.frequency_coarse - 300000; i < frequency_rssi.frequency_coarse + 300000; i += 20000)
        {
            uint32_t frequency = i;
            ELECHOUSE_cc1101.setMHZ((float)frequency / 1000000.0);
            ELECHOUSE_cc1101.SetRx();
            delayMicroseconds(3500);
            rssi = ELECHOUSE_cc1101.getRssi();

            if (rssi >= frequency_rssi.rssi_coarse)
            {
                if (frequency_rssi.rssi_fine < rssi)
                {
                    frequency_rssi.rssi_fine = rssi;
                    frequency_rssi.frequency_fine = frequency;
                }
            }
        }
    }

    if (frequency_rssi.frequency_fine != 0)
    {
        // The frequency out would be approximately ±0.02 of the true frequency
        Serial.printf("1. COARSE      Frequency: %.2f  RSSI: %d\n", (float)frequency_rssi.frequency_coarse / 1000000.0, frequency_rssi.rssi_coarse);
        Serial.printf("2. FINE        Frequency: %.2f  RSSI: %d\n\n", (float)frequency_rssi.frequency_fine / 1000000.0, frequency_rssi.rssi_fine);
    }
}
