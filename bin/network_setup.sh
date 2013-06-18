#!/bin/sh

sudo ifconfig  eth0 ${1} #192.168.1.123
#sudo ifconfig eth0

#ping 192.168.1.1
sudo route add default gw 192.168.1.1
echo "echo \"nameserver 202.96.209.5\" >> /etc/resolv.conf"
#sudo echo "nameserver 202.96.209.5" >> /etc/resolv.conf
cat /etc/resolv.conf
