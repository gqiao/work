#!/bin/sh

ssh george@10.80.104.250 -f "x11vnc -auth ~/.Xauthority -rfbauth ~/.vnc/passwd -display :0"

