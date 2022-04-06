#!/usr/bin/env bash

if tipi . -t linux-cxx17 -j8; then
	mv build/linux-cxx17/bin/compile_commands.json .
	clear
else
	echo "Tipi build failed"
	exit 1
fi
