#!/bin/bash

source ./dataset_args.sh
check_dir_and_make_if_absent_notquit ../build
cd ../build
cmake ..
make -j