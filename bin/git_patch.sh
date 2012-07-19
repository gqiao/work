#!/bin/sh
U=gqiao


git fetch ssh://${U}@ti.com:29418/ics/kernel/omap refs/changes/36/1636/3 && git format-patch -1 --start-number 1 FETCH_HEAD
git fetch ssh://${U}@ti.com:29418/ics/kernel/omap refs/changes/37/1637/3 && git format-patch -1 --start-number 2 FETCH_HEAD
git fetch ssh://${U}@ti.com:29418/ics/kernel/omap refs/changes/38/1638/3 && git format-patch -1 --start-number 3 FETCH_HEAD
git fetch ssh://${U}@ti.com:29418/ics/kernel/omap refs/changes/39/1639/3 && git format-patch -1 --start-number 4 FETCH_HEAD
git fetch ssh://${U}@ti.com:29418/ics/kernel/omap refs/changes/40/1640/3 && git format-patch -1 --start-number 5 FETCH_HEAD
git fetch ssh://${U}@ti.com:29418/ics/kernel/omap refs/changes/41/1641/3 && git format-patch -1 --start-number 6 FETCH_HEAD
git fetch ssh://${U}@ti.com:29418/ics/kernel/omap refs/changes/42/1642/3 && git format-patch -1 --start-number 7 FETCH_HEAD
git fetch ssh://${U}@ti.com:29418/ics/kernel/omap refs/changes/99/1699/1 && git format-patch -1 --start-number 8 FETCH_HEAD
git fetch ssh://${U}@ti.com:29418/ics/kernel/omap refs/changes/00/1700/1 && git format-patch -1 --start-number 9 FETCH_HEAD




#git fetch ssh://gqiao@ti.com:29418/ics/repo/u-boot refs/changes/01/1701/1 && git cherry-pick FETCH_HEAD
