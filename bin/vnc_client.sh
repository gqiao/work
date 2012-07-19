#!/bin/sh

ssh -X -L 5900:localhost:5900 ${1}

