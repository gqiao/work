#!/bin/sh

ETH_NAME=eth15
USR_NAME=george

#sudo apt-get install uml-utilities bridge-utils

# vim:set ft=sh:

## 2007-09 windwiny vboxbridgedrun.sh
## 使用前自行修改 id,ip,gw,eth?,br?,tap? 等相关参数,

# VirtualBox Bridging VirtualBox 实现桥接模式
## 参考 http://www.oceanboo.cn/read.php?55

# Ubuntu 里安装软件包
## sudo apt-get install uml-utilities bridge-utils
## ---------------------------------------------------------------------

if [ "$1" = "" ]; then
    echo -e "$RED\n `basename "$0"` {start|stop} $WHTE\n"
    exit 1
fi

if [ `id -u` -ne 0 ]; then
    echo -e "$RED\n Must be root $WHTE\n"
    exit 1
fi

if [ "$1" = "start" ] ; then
        # Create a tap device with permission for the USR
        chmod 0666 /dev/net/tun

        # create tap0
        tunctl -t tap0 -u ${USR_NAME} 
        ifconfig tap0 0.0.0.0 promisc

	# create tap1
	tunctl -t tap1 -u ${USR_NAME} 
        ifconfig tap1 0.0.0.0 promisc

        ifconfig ${ETH_NAME} 0.0.0.0 promisc

        # Create a new bridge(br0) 
        brctl addbr br0

	# add the interfaces to the bridge.
        brctl addif br0 ${ETH_NAME}
        brctl addif br0 tap0
        brctl addif br0 tap1

        # 下面是两种获取IP的方式，可以自由选择，把不需要的注释掉就好了。
        # [1] IP STATIC
        #     ifconfig br0 192.168.1.243 netmask 255.255.255.0 up
        #     route add default gw 192.168.1.1
        # [2] IP DHCP
        dhclient br0
fi


if [ "$1" = "stop" ] ; then
        # delete tap
        tunctl -d tap0
        tunctl -d tap1

        # delete br0
        ifconfig br0 down
        brctl delbr br0
        
        # 将tap0, eth0 移出bridge(br0)
        brctl delif br0 tap0
        brctl delif br0 tap1
        brctl delif br0 ${ETH_NAME}

        # [1] IP STATIC
        #     ifconfig eth0 192.168.1.243 netmask 255.255.255.0 up
        #     route add default gw 192.168.1.1
	# [2] IP DHCP 
        dhclient ${ETH_NAME}
fi



