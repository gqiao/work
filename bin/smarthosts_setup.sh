#!/bin/sh

if [ ! -e /etc/hosts.bak ]; then
    sudo cp /etc/hosts /etc/hosts.bak
fi

wget https://smarthosts.googlecode.com/svn/trunk/hosts -O /tmp/hosts
sed "s/localhost/localhost `hostname`/g" /tmp/hosts > /tmp/etc_hosts
sudo cp -fr /tmp/etc_hosts /etc/hosts

cat /etc/hosts
