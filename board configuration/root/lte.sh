#!/bin/bash

EXPORT="/sys/class/gpio/export"
AIR="/sys/devices/platform/gpio-sunxi/gpio/gpio15_pc19"
RESET="/sys/devices/platform/gpio-sunxi/gpio/gpio2_pb4"
POWER="/sys/devices/platform/gpio-sunxi/gpio/gpio16_pc7"

for i in  2 15 16;
do
         echo $i > $EXPORT
done

echo out > $AIR/direction
echo out > $RESET/direction
echo out > $POWER/direction
echo 0 > $AIR/value
echo 1 > $RESET/value
echo 1 > $POWER/value

