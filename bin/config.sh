# This file is not executable: it is sourced from the other scripts.
# Edit this file to set up your configuration.


THREADS=9
if [ "$THREADS" -lt 1 -o "$THREADS" -gt 12 ]; then
	echo "$prog: \"config.sh\" has failed to detect the number of CPUs."
	echo "Edit it and set THREADS manually."
	exit 1
fi


function banner {
	echo ""
	echo "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"
	echo ">> $*"
	echo "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
	echo ""
}


function fail {
	echo "$1 failed"
	exit $2
}  
          

function cdir {
	name=$1
	if [ ! -e $name ]; then
		echo "No such directory: $name"
		exit 18
	fi
	if [ ! -d $name ]; then
		echo "Not a directory: $name"
		exit 19
	fi
	cd $name || fail "cd $name" $?
}


function ndir {
	name=$1
	if [ ! -e $name ]; then
		mkdir --parents $name || fail "mkdir $name" $?
	fi
	if [ ! -d $name ]; then
		echo "Not a directory: $name"
		exit 20
	fi
	cd $name || fail "cd $name" $?
}


# mountpart vol-label mount-point
function mountpart {
	label=$1
	mount=$2
	if [ ! -d ${mount} ]; then
		simpcmd "Make ${mount}" mkdir ${mount}
	fi
	simpcmd "Mount ${label}" sudo mount -L ${label} ${mount}
}


function simpcmd {
	name=$1
	shift
	$* || fail "$name" $?
}
          

function logcmd {
	name=$1
	shift
	log="$LOGDIR/${name}.log"
	echo "Build: $name" > "$log"
	echo "    `pwd`" >> "$log"
	echo "    `date`" >> "$log"
	echo "" >> "$log"
	$* 2>&1 | tee -a "$log"
	status=${PIPESTATUS[0]}
	if [ $status != 0 ]; then
		echo "" >> "$log"
		echo "FAILED" >> "$log"
		echo ""
		echo "$name FAILED"
		exit $status
	fi
	echo "" >> "$log"
	echo "OK" >> "$log"
}
