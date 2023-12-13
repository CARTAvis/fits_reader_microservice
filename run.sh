#!/bin/bash

for i in {8080..8095}
do
    ./cmake-build-release/worker/fits_read ${i} &
done
