# Analyze frequencies using CC1101

[CC1101 Module](https://a.aliexpress.com/_mK4kt1M) 

[CC1101 Library by jgromes](https://github.com/jgromes/RadioLib)

[Modified to work with RadioLib Arduino CC1101 library from Flipper Zero firmware](https://github.com/flipperdevices/flipperzero-firmware/blob/ab2fcaf4abe4d1f235ad6fda7216373cf22f20ba/applications/main/subghz/helpers/subghz_frequency_analyzer_worker.c)

Try this [frequency list](https://github.com/flipperdevices/flipperzero-firmware/blob/ab2fcaf4abe4d1f235ad6fda7216373cf22f20ba/applications/debug/subghz_test/helpers/subghz_test_frequency.c) if what you need is outside the default range.

You may need to comment [this](https://github.com/jgromes/RadioLib/blob/41f89198aac4974e64b01f283792d7ef06f04890/src/modules/CC1101/CC1101.cpp#L431) to suppress the error thrown by RadioLib.

## Example output

```
Frequency Analayzer Started!
COARSE      Frequency: 314.98  RSSI: -53
COARSE      Frequency: 314.98  RSSI: -52
FINE        Frequency: 433.83  RSSI: -74
COARSE      Frequency: 433.89  RSSI: -32
FINE        Frequency: 433.94  RSSI: -32
FINE        Frequency: 433.93  RSSI: -34
COARSE      Frequency: 915.00  RSSI: -77
COARSE      Frequency: 915.00  RSSI: -70
FINE        Frequency: 915.04  RSSI: -70
```
