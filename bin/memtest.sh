#!/bin/sh

DEVMEM=./devmem
addr=$((${START}))
end_addr=$((${END}))
while [ ${addr} -le ${end_addr} ]; do
    pattern=${addr}
    ${DEVMEM} ${addr} 32 ${pattern}
    value=$((`${DEVMEM} ${addr} 32`))
    if [ "${pattern}" = "${value}" ]; then
	echo "rw ${addr} pass"
    else
	echo "rw ${addr} failed: ${pattern} != ${value}"
    fi
done

