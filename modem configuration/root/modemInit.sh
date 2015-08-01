#!/bin/bash

DevicePath=/dev/ttyACM0
keepTrying=true

while $keepTrying; do

  if [ -e "$DevicePath" ]
    then
    keepTrying=false
    echo "Modem exist"
  fi
sleep 1

done

sleep 8

echo AT!="addscanband 3">$DevicePath
sleep 4
echo AT!="addscanband 7">$DevicePath
sleep 4
echo AT!="addscanband 20">$DevicePath
sleep 4
echo AT+CFUN=1>$DevicePath

