# UA3REO HF/VHF/UHF/SHF CW Beacon based on MAX2870 and STM32F103C8

May work on mutiband 12m,10m,6m,4m,2m,70cm,23cm,13cm,6cm,3cm bands (23.5Mhz to 10Ghz) (10Ghz on harmonics mode). Transmit custom CW message and switch to next band.

See scheme.png to connection circuit 

See STM32/Core/Inc/config.h for configuration

Use Keil IDE for compile project or use pre-build firmwares

By default, the beacon cycles through frequencies sequentially.

* If connect B11 to GND, Frequency only 2m
* If connect B10 to GND, Frequency only 70cm
* If connect B1 to GND, Frequency only 23cm
* If connect B0 to GND, Frequency only 6cm
* If connect A2 to GND, Frequency only 3cm
* If connect A0 to GND, CW message replaced by long dashes
