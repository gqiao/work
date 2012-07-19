#!/bin/sh

X&

xauth add :0 . `mcookie`

xterm -display :0

twm&

dtwm&

dtsession&

olwm&

