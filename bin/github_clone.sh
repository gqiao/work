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

if [ ! -e ~/.gitconfig ]; then
    github.sh
fi

git_clone_pull "https://github.com/gqiao" "geiser sbcl iolib maxima clack caveman bordeaux-threads hunchentoot rfc2388 cl-plus-ssl trivial-backtrace cl-base64 usocket cl-fad teepeedee2 CLSQL GSLL"

git_clone_pull "git://git.sv.gnu.org" "guile guile-lib guile-reader guile-gnome guile-avahi guile-rpc libchop skribilo lilypond"

git_clone_pull "git://git.savannah.gnu.org" "autogen guile-cairo guile-ncurses gnutls"

git_clone_pull "git://gitorious.org/guile-sdl"          "guile-sdl"
git_clone_pull "git://gitorious.org/guile-syntax-parse" "guile-syntax-parse"
git_clone_pull "git://gitorious.org/glow"               "ragnarok"

git_clone_pull "git://git.gnome.org" "beast"



