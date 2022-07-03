#STM32F1_irdecode

This code runs on a STM32F103C8T6 module. It is used to extract the timing info of IR output from TL1838.
The base source code of the firmware is from the libopencm3 USB CDC example. The example code is modified to include the timing extractor using STM32F103 timer.

An example of the output through the serial port is as below

There are 5 columns in the output

```
Column 1 : Overflow counter in decimal
Column 2 : Circular queue index counter mod 256 in hex
Column 3 : Logic level, 1 = high, 0 = low
Column 4 : The cummulative microsecond counter in hex since the board is started in hex
Column 5 : The duration of the logic in microsecond in decimal
```

In example below, the interpretation of second row result is

```
0 01 0 0072b811 3353

0        : no overflow
01       : circular queue index (in hex)
0        : logic 0
0072b811 : Cummulative microsecond counter = 0072b811 (in hex) when logic 0 is detected
3353     : logic 0 was detected for 3353 microseconds
```
and for third row

```
0 02 1 0072c52a 1847

0        : no overflow
02       : circular queue index (in hex)
0        : logic 1
0072c52a : Cummulative microsecond counter = 0072b811 (in hex) when logic 0 is detected
1847     : logic 0 was detected for 1847 microseconds
```


```
=====
0 00 1 00000000 7518225
0 01 0 0072b811 3353
0 02 1 0072c52a 1847
0 03 0 0072cc61 320
0 04 1 0072cda1 547
0 05 0 0072cfc4 403
0 06 1 0072d157 1337
0 07 0 0072d690 321
0 08 1 0072d7d1 546
0 09 0 0072d9f3 402
0 0a 1 0072db85 475
0 0b 0 0072dd60 399
0 0c 1 0072deef 471
0 0d 0 0072e0c6 323
0 0e 1 0072e209 546
0 0f 0 0072e42b 323
0 10 1 0072e56e 546
0 11 0 0072e790 323
0 12 1 0072e8d3 529
0 13 0 0072eae4 347
0 14 1 0072ec3f 547
0 15 0 0072ee62 323
0 16 1 0072efa5 546
0 17 0 0072f1c7 323
0 18 1 0072f30a 546
0 19 0 0072f52c 322
0 1a 1 0072f66e 554
```

If there is no IR transmission, TL1838 will be high. From above example, TL1838 output is

```
3353 us low (IR pulses detected)
1847 us high (no IR pulses detected)
320 us low
547 us high
...
```


#Connections

TL1838 modules have 3 pins

1. power
2. gnd
3. data output

Connect the power properly to the TL1838 module and connect data output to PA10 of STM32F103C8 module

If TL1838 module is not available, you can test out this firmware by looping back PB10 to PA10.
PB10 is a test PWM output that can generate pulses to input PA10. The pulses are too fast for current code and you will notice that the overflow counter will increase periodically.


#Building the firmware

```
$ git clone --recursive https://github.com/h0wch33/stm32f1_irdecode.git
$ cd stm32f1_irdecode
$ make -C libopencm3 TARGETS='stm32/f1'
$ make -C Release all
```



#Flashing the firmware to MCU


Use openocd to flash the firmware to MCU (change the script files path as appropriate).

A compiled copy of the firmware is included in the Release folder.


```
$ openocd -f /usr/local/share/openocd/scripts/interface/stlink.cfg -f /usr/local/share/openocd/scripts/target/stm32f1x.cfg -c "program Release/stm32f1_irdecode.bin verify reset exit 0x8000000"
```

#Using the firmware

After wiring up the IR receiver module (TL1838) or by looping back PB10 to PA10, plug the STM32F103C8 module into the USB port. A USB CDC device will be created. Run the following command to get the printout to console. In my environment, the USB serial port created is at /dev/ttyACM0.

```
#cat /dev/ttyACM0
```

Get a IR remote control and point it to the IR receiver and press some buttons. The output shown above will be generated.

#Something to note

1. Careful of missing pulses in the output result. The code is not very robust yet. Nevertheless, the IR pulses seems to be extracted well.
2. Take note of overflow in case pulses are too fast to be extracted.
3. Be diligent in interpreting the output.
4. The firmware can be used also to measure pulse timing of arbitary slow square wave.


