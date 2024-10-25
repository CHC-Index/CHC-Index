#!/bin/bash

CURRENT_DIR=$(cd "$(dirname "$0")" && pwd)
cd ${CURRENT_DIR}
set -e

source ./dataset_args.sh
source ./algo_args.sh

$1 # dataset name
$2 # algorithm name

check_dir_and_make_if_absent_notquit ../build
cd ../build
cmake ..
make -j
ln -sf ../scripts/SEA_test.jar ./

EXP_FOLDER_SUFFIX=

root="${CURRENT_DIR}/../exp_data/"
############ BUILD ################
IDX_ALGO=$2
if [ ${IDX_ALGO} == "rKACS" ]; then
  IDX_ALGO=ATC
  fi
BUILD_THREAD_NUM=60
INDEX_PREFIX="${root}/indices/${DATASET}"
INDEX_PATH=${INDEX_PREFIX}"/${IDX_ALGO}_L${BUILD_L}_R${DEGREE}_alp${ALPHA}_beta${BETA}.chci"
INDEX_LOG_PATH=${INDEX_PREFIX}"/log/${IDX_ALGO}_L${BUILD_L}_R${DEGREE}_alp${ALPHA}_beta${BETA}.chci"
BASE_INDEX_PATH=${INDEX_PATH}

############ RETRIEVE ##############
LAYER=15
QUERY_PATH_PREFIX=${root}/..//datasets/query/${DATASET}/
QUERY_PATH=${QUERY_PATH_PREFIX}/${DATASET}_queries_${LAYER}${COMBASE}

rKACS_out=0
if [ $2 == rKACS ]; then
  rKACS_out=1
fi

############### Retrieve Param ######################
DEFAULT_THREAD_NUM=6
RETRIEVE_RES_PREFIX="${root}/retrieve/${EXP_FOLDER_SUFFIX}/${DATASET}/"
RETRIEVE_RES_PATH="${RETRIEVE_RES_PREFIX}${2}_r${r}_layer${LAYER}.log"
RETRIEVE_RES_LOG_PATH="${RETRIEVE_RES_PREFIX}log/${2}_$4_r${r}_layer${LAYER}.log"

############ VAC ###################
VAC_PREFIX="${root}vac/${EXP_FOLDER_SUFFIX}/${DATASET}/"
VAC_RES_PATH="${VAC_PREFIX}with_r${r}_layer${LAYER}.log"
VAC_RES_PATH_WO="${VAC_PREFIX}wo_r${r}_layer${LAYER}.log"
VAC_RUN_WITH=1
VAC_RUN_WO=0

############ SEA ###################
SEA_PREFIX="${root}sea/${EXP_FOLDER_SUFFIX}/${DATASET}/"
SEA_RES_PATH_WITH="${SEA_PREFIX}with_r${r}_layer${LAYER}_add${VALID_NODE_PERCENT}.log"
SEA_RES_PATH_WO="${SEA_PREFIX}wo_r${r}_layer${LAYER}.log"
SEA_RUN_WO=1

############ ATC ###################
ATC_d=3
ATC_perc=0.50
ATC_RUN_WITH=1
ATC_RUN_WO=1

ATC_PREFIX="${root}atc/${EXP_FOLDER_SUFFIX}/${DATASET}/"
ATC_RES_PATH="${ATC_PREFIX}with_r${r}_layer${LAYER}_add_${VALID_NODE_PERCENT}.log"
ATC_RES_PATH_WO=${root}atc/${DATASET}/WO/wo_atcperc_${ATC_perc}_atcd_${ATC_d}.log
############ rKACS ##################
rKACS_PREFIX="${root}rKACS/${EXP_FOLDER_SUFFIX}/${DATASET}/"
rKACS_RES_PATH_WITH="${rKACS_PREFIX}with_r${r}_layer${LAYER}_dschange${VALID_NODE_PERCENT}.log"
rKACS_RES_PATH_WO="${rKACS_PREFIX}wo_layer${LAYER}_dschange${VALID_NODE_PERCENT}.log"
rKACS_FILE_PREFIX="${root}/..//datasets/rKACS/${DATASET}/"
rKACS_TRUSSNESS_FILE=${rKACS_FILE_PREFIX}${DATASET}_trussness_origin.txt
rKACS_SIMILARITY_FILE=${rKACS_FILE_PREFIX}${DATASET}_similarity_origin.txt
VALID_NODE_PATH=null
rKACS_RUN_WO=0

if [ ${VALID_NODE_PERCENT} != 1 ]; then
  rKACS_RUN_WO=0
fi
set -x
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
    check_dir_and_make_if_absent_notquit ${RETRIEVE_RES_PREFIX}

  ./Retrieve ${DATASET}  ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${RETRIEVE_RES_PATH} ${RETRIEVE_RES_LOG_PATH} ${QUERY_PATH}\
            ${r} ${LAYER} ${COMBASE} ${SMETHOD} ${METRIC} ${DEFAULT_THREAD_NUM} ${rKACS_out}
  ;;
run)
  case $2 in
 VAC)
     check_dir_and_make_if_absent_notquit ${VAC_PREFIX}
   ./VAC_test ${DATASET}  ${BASE_INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${RETRIEVE_RES_PATH} ${VAC_RES_PATH} ${VAC_RES_PATH_WO} \
               ${LAYER} ${VAC_RUN_WITH} ${VAC_RUN_WO} 1
   ;;
 ATC)
     check_dir_and_make_if_absent_notquit ${ATC_PREFIX}

   ./ATC_test ${DATASET}  ${BASE_INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${RETRIEVE_RES_PATH} ${ATC_RES_PATH} ${ATC_RES_PATH_WO}\
               ${LAYER} ${ATC_d} ${ATC_perc} ${ATC_RUN_WITH} ${ATC_RUN_WO} ${DEFAULT_THREAD_NUM}
   ;;
 SEA)
     check_dir_and_make_if_absent_notquit ${SEA_PREFIX}
   java -jar ./SEA_test.jar k-core ${DATASET} ${DATASET_TYPE} ${RETRIEVE_RES_PATH} ${LAYER} ${META_PATH} ${SEA_RUN_WO} ${SEA_RES_PATH_WITH} ${SEA_RES_PATH_WO} \
               ${graph} ${vertex} ${edge} ${data} ${VALID_NODE_PATH}
               ;;
 rKACS)
     check_dir_and_make_if_absent_notquit ${rKACS_PREFIX}
     check_dir_and_make_if_absent_notquit ${rKACS_FILE_PREFIX}
     ./rKACS_Query ${DATASET} ${graph} ${vertex} ${edge} ${data} ${VALID_NODE_PATH} ${rKACS_TRUSSNESS_FILE} ${rKACS_SIMILARITY_FILE}\
                   ${LAYER} 1 ${RETRIEVE_RES_PATH} ${rKACS_RES_PATH_WITH} ${rKACS_RES_PATH_WO} ${rKACS_RUN_WO}
     ;;
   esac
  ;;
esac
set +x
