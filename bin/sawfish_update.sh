#!/bin/sh

cd /work/src/sawfish

git_clone_pull() {
    URL=${1}
    DIRNAME=${2}

    if [ ! -e ${DIRNAME} ]; then
        git clone ${URL} ${DIRNAME}
    fi
    cd ${DIRNAME}
    git pull
    cd - > /dev/null
}

git_clone_pull git://git.tuxfamily.org/gitroot/librep/main.git        librep
git_clone_pull git://git.tuxfamily.org/gitroot/librep/gtk.git         rep-gtk
git_clone_pull git://git.tuxfamily.org/gitroot/sawfish/main.git       sawfish
git_clone_pull git://git.tuxfamily.org/gitroot/sawfish/ssd.git        sawfish-session-dialog
git_clone_pull git://git.tuxfamily.org/gitroot/sawfishpager/pager.git sawfish-pager








