#!/bin/sh

#YOUR_LOGNAME=george
#YOUR_IP=10.237.60.67

if [ ! -f ~/.ssh/id_rsa.pub ]; then
    ssh-keygen -t rsa 
fi

ssh-copy-id -i ~/.ssh/id_rsa.pub ${1}




