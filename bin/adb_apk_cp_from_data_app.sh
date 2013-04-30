#!/bin/bash

data=`adb shell "ls /data/app"`

idx=0
for i in ${data}; do
    echo "${idx}\t${i}"
    items="${items} ${idx} ${i}"
    idx=$((${idx} + 1))
done


dialog --menu "Change Enviroment ?" 40 90 40 ${items} 2>K.txt
K=$(cat K.txt)
echo "K=${K}"

if [ -z ${K} ]; then
    echo "DO NOTHING !!!" | figlet
    return
fi

idx=0
for i in ${data}; do
    if [ ${K} = ${idx} ]; then
        break
    fi
    idx=$((${idx} + 1))
done

echo "Please use this command:"
echo "    adb pull /data/app/${i}"




