#!/bin/bash

S=/work/src/${TARGET_NAME}.${BRANCH_NAME}.${SERVER_NAME}
cd ${S}

LOGDIR=buildlogs/`date +%y%m%d-%H%M%S`
mkdir -p ${LOGDIR}
log="${LOGDIR}/build.log"

OS=`uname`
if [ ${OS} = "Darwin" ]; then
    THREADS=9

    if cat device/ti/common-open/board_identity/src/generate_info.sh | grep -q "sort -Vr"; then
	cp device/ti/common-open/board_identity/src/generate_info.sh /tmp/generate_info.sh
	sed 's/sort -Vr/sort -r/g' /tmp/generate_info.sh > device/ti/common-open/board_identity/src/generate_info.sh
    fi
else
    THREADS=$((`cat /proc/cpuinfo | grep processor | wc -l` + 1))
fi

source build/envsetup.sh
lunch ${TARGET_NAME}-userdebug
make -j${THREADS} 2>&1 | tee -a ${log}



