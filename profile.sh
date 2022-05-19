#!/bin/bash

profile=profiles/$(date +%s)
mkdir -p profiles
valgrind --tool=callgrind --callgrind-out-file=$profile.out ./Overhead.exe
callgrind_annotate --auto=yes ./$profile.out > $profile.txt