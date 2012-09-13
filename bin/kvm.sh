#!/bin/sh

cd /work/src/kvm

git_clone_pull() {
    URL=${1}
    REPOS=${2}

    for i in ${REPOS}; do
        echo "[ ${i} ]"
        if [ ! -e ${i} ]; then
            git clone ${URL}/${i}.git 
        fi
        if [ -e ${i} ]; then
            cd ${i}
            git pull
            cd - > /dev/null
        fi
    done
}

# [kernel kvm] [kernel module kvm] [userspace kvm]
git_clone_pull "git://git.kernel.org/pub/scm/virt/kvm" "kvm kvm-kmod qemu-kvm" 
# [kernel module vfio]
git_clone_pull "git://github.com/awilliam"             "linux-vfio"
# [kernel kvm arm] [usersapce kvm arm]
git_clone_pull "git://github.com/virtualopensystems"   "linux-kvm-arm qemu" 

