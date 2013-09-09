#!/bin/sh


usage() {
    echo "$0 -t|--try -h|--help"
    echo "Example:"
    echo "$0 --try firstname lastname 01 02 03 04 05 06 07 08 09 10 11 12"
    echo "$0       firstname lastname 01 02 03 04 05 06 07 08 09 10 11 12"
}

if [ -z ${2} ]; then
    usage
    exit 1
fi

TRY="no"

args=`getopt -o ht: -l help,try: -- "$@"`

while true; do
    case "${1}" in
    -t|--try)
        TRY="yes"
        shift ;;
    -h|--help)
        usage
        exit 0 ;;
    *) break ;;
    esac
done



firstnewname=$1
shift
lastnewname=$1
shift

echo "newname = ${firstnewname}XXX${lastnewname}"
echo "\$@ = ${@}"
# $@ = 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30


for i in $@; do
    filename=`find . -name "*${i}.mp3"`
    #ls -l "${filename}"
    if [ "${TRY}" = "no" ]; then
        echo "try is no , so do it!"
        mv "${filename}" "${firstnewname}${i}${lastnewname}"
    else
        echo "oldname = ${filename} newname = ${firstnewname}${i}${lastnewname}"
    fi
done
