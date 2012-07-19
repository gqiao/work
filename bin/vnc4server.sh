#!/bin/sh

vnc4server -kill :${1}

# restart vnc server
vnc4server



