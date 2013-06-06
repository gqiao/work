#!/bin/sh

if [ ! -e /etc/hosts.bak ]; then
    sudo cp /etc/hosts /etc/hosts.bak
fi

sudo wget https://smarthosts.googlecode.com/svn/trunk/hosts -O /etc/hosts

# sudo wget https://smarthosts.googlecode.com/svn/trunk/hosts
# sudo mv ./hosts /etc/hosts
cat /etc/hosts

