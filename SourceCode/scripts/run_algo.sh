#!/bin/bash

set -e

source ./dataset_args.sh
source ./algo_args.sh

$1 # dataset name
$2 # algorithm name

check_dir_and_make_if_absent_notquit ../build
cd ../build
cmake ..
make -j
ln -sf ../shells/SEA_test.jar ./


#root="../exp_data/"
root="$(pwd)/../exp_data/"
############ BUILD ################
INDEX_PREFIX="${root}/indices/${DATASET}"
INDEX_PATH=${INDEX_PREFIX}"/${2}_L${BUILD_L}_R${DEGREE}_alp${ALPHA}_beta${BETA}.chci"
INDEX_LOG_PATH=${INDEX_PREFIX}"/log/${2}_L${BUILD_L}_R${DEGREE}_alp${ALPHA}_beta${BETA}.chci"

############ PART BUILD ################
BUILD_THREAD_NUM=6
PRUNE_MAX_LAYER=999
Prune_Root=${root}/Prune/${DATASET}
VALID_NODE_PERCENT=5
PART_INDEX_PREFIX="${root}/part_indices/${DATASET}/base/"
PART_INDEX_PATH=${PART_INDEX_PREFIX}"/${DATASET}_$2_L${BUILD_L}_R${DEGREE}_alp${ALPHA}_beta${BETA}_del${VALID_NODE_PERCENT}.chci"
PART_INDEX_LOG_PATH=${PART_INDEX_PREFIX}"/log/${DATASET}_L${BUILD_L}_R${DEGREE}_alp${ALPHA}_beta${BETA}_del${VALID_NODE_PERCENT}.chci"
VALID_NODE_PATH=${Prune_Root}/${DATASET}_remain_${VALID_NODE_PERCENT}%.txt

# 加到多少百分比
ADD_START_PERCENT=5
ADD_START=${PART_INDEX_PATH}
ADD_NODES_PATH_MULTI=""
ADD_INDEX_LOG_PATH_MULTI=""

for ADD_PERCENT in 1 2 4 8 16 32
do
  ADD_NODES_PATH=${Prune_Root}/${DATASET}_del_${ADD_PERCENT}%.txt
  ADD_INDEX_PREFIX=${PART_INDEX_PREFIX}/Add/
  ADD_INDEX_LOG_PREFIX=${PART_INDEX_PREFIX}/Add/log/
  ADD_INDEX_PATH=${ADD_INDEX_PREFIX}/${DATASET}_$2_add_${ADD_PERCENT}%.txt
  ADD_INDEX_LOG_PATH=${ADD_INDEX_LOG_PREFIX}/${DATASET}_$2_add_${ADD_TO_PERCENT}%.txt

  ADD_NODES_PATH_MULTI=${ADD_NODES_PATH},${ADD_NODES_PATH_MULTI}
  ADD_INDEX_PATH_MULTI=${ADD_INDEX_PATH},${ADD_INDEX_PATH_MULTI}
done
echo $ADD_INDEX_PATH_MULTI
echo $ADD_NODES_PATH_MULTI

############ RETRIEVE ##############
r=0.15
LAYER=4
queryID=20
rKACS_out=0
RETRIEVE_RES_PREFIX="${root}retrieve/${DATASET}/"
RETRIEVE_RES_PATH="${RETRIEVE_RES_PREFIX}${2}_r${r}_layer${LAYER}.log"
RETRIEVE_RES_LOG_PATH="${RETRIEVE_RES_PREFIX}log/${2}_r${r}_layer${LAYER}.log"
if [ $2 == rKACS ]; then
  rKACS_out=1
fi

############ VAC ###################
VAC_PREFIX="${root}vac/${DATASET}/"
VAC_RES_PATH="${VAC_PREFIX}with_r${r}_layer${LAYER}.log"
VAC_RES_PATH_WO="${VAC_PREFIX}wo_r${r}_layer${LAYER}.log"
VAC_RUN_WITH=1
VAC_RUN_WO=1

############ ATC ###################
ATC_PREFIX="${root}atc/${DATASET}/"
ATC_RES_PATH="${ATC_PREFIX}with_r${r}_layer${LAYER}.log"
ATC_d=3
ATC_gamma=0.2

############ SEA ###################
SEA_PREFIX="${root}sea/${DATASET}/"
SEA_RES_PATH_WITH="${SEA_PREFIX}with_r${r}_layer${LAYER}.log"
SEA_RES_PATH_WO="${SEA_PREFIX}wo_r${r}_layer${LAYER}.log"
SEA_RUN_WO=1

############ rKACS ##################
rKACS_PREFIX="${root}rKACS/${DATASET}/"
rKACS_RES_PATH_WITH="${rKACS_PREFIX}with_r${r}_layer${LAYER}.log"
rKACS_RES_PATH_WO="${rKACS_PREFIX}wo_r${r}_layer${LAYER}.log"
rKACS_RUN_WO=1

case $3 in
  build)
    check_dir_and_make_if_absent_notquit ${INDEX_PREFIX}/log
    echo "Building ..."
    echo "./Build ${DATASET} ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${DEGREE} ${BUILD_L} \
                 ${ALPHA} ${BETA} ${RPPP} ${BUILD_THREAD_NUM} ${COMBASE} ${SMETHOD} ${METRIC}"
    ./Build ${DATASET} ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${DEGREE} ${BUILD_L} \
       ${ALPHA} ${BETA} ${RPPP} ${BUILD_THREAD_NUM} ${COMBASE} ${SMETHOD} ${METRIC}
   ;;
retrieve)
    check_dir_and_make_if_absent_notquit ${RETRIEVE_RES_PREFIX}/log
    echo "./Retrieve ${DATASET}  ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${RETRIEVE_RES_PATH} ${RETRIEVE_RES_LOG_PATH} ${queryID} \
                      ${r} ${LAYER} ${COMBASE} ${SMETHOD} ${METRIC} 1 ${rKACS_out}"
  ./Retrieve ${DATASET}  ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${RETRIEVE_RES_PATH} ${RETRIEVE_RES_LOG_PATH} ${queryID} \
            ${r} ${LAYER} ${COMBASE} ${SMETHOD} ${METRIC} 1 ${rKACS_out}
  ;;
VAC)
    check_dir_and_make_if_absent_notquit ${VAC_PREFIX}
  echo "./VAC_test ${DATASET}  ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${RETRIEVE_RES_PATH} ${VAC_RES_PATH} ${VAC_RES_PATH_WO}\
                ${LAYER} ${VAC_RUN_WITH} ${VAC_RUN_WO} 1 "
  ./VAC_test ${DATASET}  ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${RETRIEVE_RES_PATH} ${VAC_RES_PATH} ${VAC_RES_PATH_WO} \
              ${LAYER} ${VAC_RUN_WITH} ${VAC_RUN_WO} 1
  ;;
ATC)
    check_dir_and_make_if_absent_notquit ${ATC_PREFIX}
  echo "./ATC_test ${DATASET}  ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${RETRIEVE_RES_PATH} ${ATC_RES_PATH} \
                ${LAYER} ${ATC_d} ${ATC_gamma} ${VAC_RUN_WITH} ${VAC_RUN_WO} 1"
  ./ATC_test ${DATASET}  ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${RETRIEVE_RES_PATH} ${ATC_RES_PATH} \
              ${LAYER} ${ATC_d} ${ATC_gamma} ${VAC_RUN_WITH} ${VAC_RUN_WO} 1
  ;;
SEA)
    check_dir_and_make_if_absent_notquit ${SEA_PREFIX}
  echo "./SEA_test.jar k-core ${DATASET} ${DATASET_TYPE} ${RETRIEVE_RES_PATH} ${LAYER} ${META_PATH} ${SEA_RUN_WO} ${SEA_RES_PATH_WITH} ${SEA_RES_PATH_WO} \
                ${graph} ${vertex} ${edge} ${data}"
  java -jar ./SEA_test.jar k-core ${DATASET} ${DATASET_TYPE} ${RETRIEVE_RES_PATH} ${LAYER} ${META_PATH} ${SEA_RUN_WO} ${SEA_RES_PATH_WITH} ${SEA_RES_PATH_WO} \
              ${graph} ${vertex} ${edge} ${data}
              ;;
rKACS)
    check_dir_and_make_if_absent_notquit ${rKACS_PREFIX}
    echo "./rKACS_Query ${DATASET} q kr ${LAYER} 1 ${RETRIEVE_RES_PATH} ${rKACS_RES_PATH_WITH} ${rKACS_RES_PATH_WO} ${rKACS_RUN_WO}"
    ./rKACS_Query ${DATASET} q kr ${LAYER} 1 ${RETRIEVE_RES_PATH} ${rKACS_RES_PATH_WITH} ${rKACS_RES_PATH_WO} ${rKACS_RUN_WO}
    ;;
Prune)
  check_dir_and_make_if_absent_notquit ${Prune_Root}
  echo "./Prune ${DATASET}  ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${PRUNE_MAX_LAYER} ${COMBASE} ${SMETHOD} ${METRIC} ${Prune_Root}"
  ./Prune ${DATASET}  ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${PRUNE_MAX_LAYER} ${COMBASE} ${SMETHOD} ${METRIC} ${Prune_Root}
    ;;
PartBuild)
    check_dir_and_make_if_absent_notquit ${PART_INDEX_PREFIX}/log
    echo "Building ..."
    echo "./Part_Build ${DATASET} ${PART_INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${VALID_NODE_PATH} ${DEGREE} ${BUILD_L} \
                 ${ALPHA} ${BETA} ${RPPP} ${BUILD_THREAD_NUM} ${COMBASE} ${SMETHOD} ${METRIC}"
     ./Part_Build ${DATASET} ${PART_INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${VALID_NODE_PATH} ${DEGREE} ${BUILD_L} \
                      ${ALPHA} ${BETA} ${RPPP} ${BUILD_THREAD_NUM} ${COMBASE} ${SMETHOD} ${METRIC}
   ;;
AddNodes)
    check_dir_and_make_if_absent_notquit ${ADD_INDEX_LOG_PREFIX}
    echo "Building ..."
    echo "./Add_Nodes ${DATASET} ${ADD_START} ${graph}  ${vertex} ${edge} ${data} ${VALID_NODE_PATH} ${ADD_NODES_PATH} ${ADD_INDEX_PATH} ${DEGREE} ${BUILD_L} \
                 ${ALPHA} ${BETA} ${RPPP} ${BUILD_THREAD_NUM} ${COMBASE} ${SMETHOD} ${METRIC}"
   ./Add_Nodes_Multi ${DATASET} ${ADD_START} ${graph}  ${vertex} ${edge} ${data} ${VALID_NODE_PATH} ${ADD_NODES_PATH_MULTI} ${ADD_INDEX_PATH_MULTI} ${DEGREE} ${BUILD_L} \
                    ${ALPHA} ${BETA} ${RPPP} ${BUILD_THREAD_NUM} ${COMBASE} ${SMETHOD} ${METRIC}
   ;;

esac