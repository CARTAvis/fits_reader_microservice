#!/bin/bash

for i in {8080..8111}
do
    ./cmake-build-release/worker/fits_read ${i} &
done
