#!/bin/bash

CURRENT_DIR=$(cd "$(dirname "$0")" && pwd)
cd ${CURRENT_DIR}

source ./dataset_args.sh
# Build snap library
cd ../3rdPartyLib/snap
make all

cd ${CURRENT_DIR}
check_dir_and_make_if_absent_notquit ../build
cd ../build
cmake ..
make -j
