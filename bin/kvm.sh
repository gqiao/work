#!/bin/sh

git_clone_pull() {
    URL=${1}
    REPOS=${2}

    for i in ${REPOS}; do
        echo "[ ${i} ]"
        if [ ! -e ${i} ]; then
            git clone ${URL}/${i}.git
        fi
        cd ${i}
        git pull
        cd - > /dev/null
    done
}

echo "[kernel kvm tree]"
echo "[kernel module kvm tree]"
echo "[userspace kvm tree]"
git_clone_pull "git://git.kernel.org/pub/scm/virt/kvm" "kvm" "kvm-kmod" "qemu-kvm" 


git_clone_pull "git://github.com/awilliam" "linux-vfio"

