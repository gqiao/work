#!/bin/sh

RELEASE=precise
#{lucid, maverick, natty, precise}

# if firewall
# git clone http://kernel.ubuntu.com/git-repos/ubuntu/ubuntu-${RELEASE}.git
# else
git clone git://kernel.ubuntu.com/ubuntu/ubuntu-${RELEASE}.git
# endif

#git fetch
#git rebase --onto origin/master origin/master@{1}


