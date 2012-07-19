#!/bin/sh

ETH_NAME=eth7
IP_NAME=192.168.10.235
BR_NAME=br1
USR_NAME=george
TAP_NAME=tap1

#sudo apt-get install uml-utilities bridge-utils

# vim:set ft=sh:

## 2007-09 windwiny vboxbridgedrun.sh
## 使用前自行修改 id,ip,gw,eth?,br?,tap? 等相关参数,

# VirtualBox Bridging VirtualBox 实现桥接模式
## 参考 http://www.oceanboo.cn/read.php?55

# Ubuntu 里安装软件包
## sudo apt-get install uml-utilities bridge-utils
## ---------------------------------------------------------------------

echo "[env]"
echo "ETH_NAME = ${ETH_NAME}"
echo "IP_NAME  = ${IP_NAME}"
echo "BR_NAME  = ${BR_NAME}"
echo "USR_NAME = ${USR_NAME}"
echo "TAP_NAME = ${TAP_NAME}"

if [ "$1" = "" ]; then
    echo "usage: $RED\n `basename "$0"` {start|stop} $WHTE\n"
    exit 1
fi

if [ `id -u` -ne 0 ]; then
    echo -e "$RED\n Must be root $WHTE\n"
    exit 1
fi

if [ "$1" = "start" ] ; then
        # Create a tap device with permission for the user running vbox
        # 建立一个使用者(user)有权限的设备${TAP_NAME}，-u 参数为自己用户名 或 id
        tunctl -t ${TAP_NAME} -u ${USR_NAME}  # 不能用 `id -u`,因为使用sudo 执行时id为0
        chmod 0666 /dev/net/tun

        # Bring up ethX and tapX in promiscuous mode
        # 将ethx和tapx网卡界面设为混杂模式(Promiscuous)
        ifconfig ${ETH_NAME} 0.0.0.0 promisc
        ifconfig ${TAP_NAME} 0.0.0.0 promisc

        # Create a new bridge and add the interfaces to the bridge.
        # 建立新的桥接界面(bridge)，並把 eth0, ${TAP_NAME}加入bridge
        brctl addbr ${BR_NAME}
        brctl addif ${BR_NAME} ${ETH_NAME}
        brctl addif ${BR_NAME} ${TAP_NAME}

        # 下面是两种获取IP的方式，可以自由选择，把不需要的注释掉就好了。

	if [ "${IP_NAME}" = "dhcp" ]; then
            # [1] 将bridge设成动态DHCP分配IP。
            dhclient ${BR_NAME}
	else
            # [2] 将bridge设成静态IP。XXX都分别对应IP、子网掩码、网关。
            ifconfig ${BR_NAME} ${IP_NAME} netmask 255.255.255.0 up
            # route add default gw 192.168.1.1
	fi
fi


if [ "$1" = "stop" ] ; then
        ## 刪除 ${TAP_NAME}
        tunctl -d ${TAP_NAME}
        ##
        ## 刪除 ${BR_NAME}
        ifconfig ${BR_NAME} down
        brctl delbr ${BR_NAME}
        ##
        ## 将${TAP_NAME}, eth0 移出bridge(${BR_NAME})
        brctl delif ${BR_NAME} ${TAP_NAME}
        brctl delif ${BR_NAME} ${ETH_NAME}
	
        ## 自定义恢复IP地址，默认网关
        #ifconfig eth0 192.168.1.243 netmask 255.255.255.0 up
        #route add default gw 192.168.1.1
	if [ "${IP_NAME}" = "dhcp" ]; then
            # [1] 
            dhclient ${ETH_NAME}
	else
            # [2] 
            ifconfig ${ETH_NAME} ${IP_NAME} netmask 255.255.255.0 up
            # route add default gw 192.168.1.1
	fi
fi



