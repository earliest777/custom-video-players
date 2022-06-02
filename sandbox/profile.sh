#!/bin/bash

profile=profiles/$(date +%s)
mkdir -p profiles
valgrind --tool=callgrind --callgrind-out-file=$profile.out ./a.out
callgrind_annotate --auto=yes ./$profile.out > $profile.txt