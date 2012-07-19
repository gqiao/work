#!/usr/bin/env bash

function logcmd {
    name=$1
    shift
    if [ $DO_LOG"x" == "0x" ]; then
        echo "Executing $name"
        try_exec "$*"
        return
    fi
    logprint "-----"
    logprint "| EXEC> $name"
    logprint "| cmd>> $*"
    logprint "-----"
    logprint "Build: $name"
    logprint "       `pwd`"
    logprint "       `date`"
    logprint ""
    $* 2>&1 | tee -a "$LOGNAME"
    status=${PIPESTATUS[0]}
    if [ $status != 0 ]; then
        logprint ""
        logprint "FAILED"
        logprint ""
        logprint "$name FAILED"
        exit $status
    fi
    logprint ""
    logprint "$name SUCCEEDED"
}

function logprint {
    if [ x"$DO_LOG" != x"0" ]; then
#Just in case we've forgotten to set a logname before use
        if [ $LOGNAME"x" == "x" ]; then
            LOGNAME=${LOGDIR}/default
        fi
        echo "$*" >> "$LOGNAME"
    fi
}

# Allows us to functionally group commands in a single log
function logname {
    if [ x"$DO_LOG" != x"0" ]; then
        export LOGNAME="${LOGDIR}/$1"
    fi
}

function fail {
# There's a chance we've failed setting up logging; if so, don't print to the log
    if [ x"$DO_LOG" != x"0" -a -e "$LOGNAME" ]; then
        logprint "[$2] $1"
    fi
    echo "[$2] $1" && exit $2
}

function try_exec {
    if [ x"$DEBUG" != x"0" -o x"$VERBOSE" != x"0" ]; then
        echo "CMD>> $1"
    fi
    if [ x"$DO_LOG" != x"0" ]; then
        logprint "CMD>> $1"
    fi
    if [ x"$DEBUG" = x"0" ]; then
        $1 || fail "ERROR: Failed to execute $1" $?
    fi
}

# Usage: safe_pushd "directory"
# Attempt to pushd a given directory, and fail if if doesn't exist. 
# Can take relative or absolute paths
function safe_pushd {
    if [ -d "$1" -o -h "$1" ]; then
        pushd $1
    else
        fail "Directory $1 does not exist or is not a directory/symlink." "1"
    fi
}

# Usage: safe_cd "directory"
# Attempt to cd to a given directory and fail if it doesn't exist.
# Can take relative or absolute paths
function safe_cd {
    if [ -d "$1" -o -h "$1" ]; then
        cd $1
    else
        fail "Directory $1 does not exist or is not a directory/symlink." "1"
    fi
}

function print_info {
    if [ $DO_LOG"x" != "0x" ]; then
        logprint "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"
        logprint ">> $1 <<"
        logprint "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
    fi
    echo "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"
    echo ">> $1 <<"
    echo "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
}

function simpcmd {
    name=$1
    shift
# There's a chance we've failed setting up logging; if so, don't print to the log
    if [ x"$DO_LOG" != x"0" -a -e "$LOGNAME" ]; then
        logprint "EXEC> $*"
    fi
    $* || fail "$name" $?
}

# Usage: clean_out
# Cleans the entire $MYDROID/out directory
function clean_out {
    print_info "Clean out folder"

    logcmd "Empty 'out' folder" rm -fr $MYDROID/out

    if [ -d $MYDROID/buildlogs ]; then
        logcmd "Delete build logs" rm -fr $MYDROID/buildlogs
    fi
}

# Usage: clean_intermediate
# Cleans out anything in the out tree that we don't want to keep. 
# For now that includes the 'fetched' directory (for eventual remote handling)
# and any existing completed framework/bsp builds (framework_*, bsp_*)
function clean_intermediate {
    print_info "Cleaning intermediate files"
    
    if [ -d "$MYDROID/out" ]; then
        pushd $MYDROID/out
        for i in `/bin/ls -1 | egrep -v -e ^bsp_ -e ^framework_ -e ^fetched$`; do
            logcmd "Deleting temporary file $1" rm -rf $i
        done
        popd
    else
        try_exec "mkdir -p $MYDROID/out"
    fi
}

# Usage: clean_intree
# Runs common 'make clean'-type targets that have no prerequisites
function clean_intree {
    print_info "Executing programmatic cleans"

    # Clean x-loader
    if [ -d $MYDROID/bootable/bootloader/x-loader ]; then
        try_exec "pushd $MYDROID/bootable/bootloader/x-loader"
        logcmd "Cleaning x-loader" make distclean
        try_exec "popd"
    fi

    # Clean u-boot
    if [ -d $MYDROID/bootable/bootloader/u-boot ]; then
        try_exec "pushd $MYDROID/bootable/bootloader/u-boot"
        logcmd "Cleaning u-boot" make distclean
        try_exec "popd"
    fi

    # Clean Wifi module
    if [ -d $MYDROID/hardware/ti/wlan/mac80211/compat -a -f $MYDROID/kernel/android-3.0/compat_version ]; then
        try_exec "pushd $MYDROID/hardware/ti/wlan/mac80211/compat"
        try_exec "make clean"
        try_exec "popd"
    fi

    # Clean kernel
    if [ -d $MYDROID/kernel/android-3.0 ]; then
        try_exec "pushd $MYDROID/kernel/android-3.0"
        logcmd "Cleaning kernel" make ARCH=${ARCH} distclean
        try_exec "popd"
    fi
}

# Usage: clean_target "directory" ("target")
# Will execute 'make (target)' in the specified directory. If 'target' is not
# defined, used 'distclean'
function clean_target {
    if [ x"$2" == "x" ]; then
        CLEAN_TARGET="distclean"
    else
        CLEAN_TARGET=$2
    fi

    if [ ! -e "$1/Makefile" ]; then
        fail "No Makefile exists in $1" "1"
    fi

    try_exec "pushd $1"
    try_exec "make $CLEAN_TARGET"
    try_exec "popd"
}

#Usage: check_dir_exist "directory"
# Will see if directory exists, and if not, will create it
function check_dir_exist {
    if [ ! -d "$1" ]; then
        if [ -e "$1" -a ! -h "$1" ]; then  #Problem: A file exists here already that's not a dir (symlinks okay)
            fail "Cannot create directory $1, a file already exists at that location." "1"
        fi
        logcmd "Creating directory $1" mkdir -p $1
    fi
}

#Export all environmental variables here so that it can be shared among
#build_bsp.sh, build_framework.sh, glue.sh or any scripts who source me
if expr `pwd` : ".*vendor\/bn\/build\/scripts$" > /dev/null; then
    cd ../../../..
fi
MYDROID=`pwd`
export CC=gcc-4.4
export CXX=g++-4.4
export PATH=${MYDROID}/bootable/bootloader/u-boot/tools:$PATH
export ARCH=arm
export CROSS_COMPILE=${ARCH}-none-linux-gnueabi-
export KERNEL_DIR=${MYDROID}/kernel/android-3.0
export KLIB=${KERNEL_DIR}
export KLIB_BUILD=${KERNEL_DIR}

#Make sure we don't redefine the logdir each and every time we include this file
if [ $DO_LIBS_SH_LOAD"x" == "x" ]; then
    export DO_LIBS_SH_LOAD=1
#Just in case we set a different LOGDIR environment variable
    if [ $LOGDIR"x" == "x" ]; then
        LOGDIR=$MYDROID  #Since this is hard-set above
    fi
    export LOGDIR=${LOGDIR}/buildlogs/`date +%y%m%d-%H%M%S`
    if [ ! -d "$LOGDIR" ]; then
        simpcmd "Make log dir" mkdir -p $LOGDIR
    fi
fi
