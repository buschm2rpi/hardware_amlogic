#!/system/bin/sh
echo off > /sys/class/gps_power/gps_powerctrl
busybox sleep 0.2
echo on > /sys/class/gps_power/gps_powerctrl
busybox sleep 0.2
