#!/bin/bash

 echo "Press Ctrl+C for quit"

 while true
 do
         vol_raw=$(cat /sys/bus/iio/devices/iio\:device0/in_voltage0_raw)
         vol_scale=$(cat /sys/bus/iio/devices/iio\:device0/in_voltage_scale)
         #vol=$(echo "scale=3.3; $vol_raw*$vol_scale/1000" | bc )
         #echo "vol_raw:$vol_raw,vol_scale:$vol_scale,vol:$vol V"
	 echo $vol_raw, $vol_scale
         sleep 1s
 done
