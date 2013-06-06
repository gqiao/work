#!/bin/sh

sudo ifconfig wlan2 up
sudo iwlist   wlan2 scan
sudo iwconfig wlan2 essid "NETGEAR" key "0000-0000-00
sudo dhclient wlan2

