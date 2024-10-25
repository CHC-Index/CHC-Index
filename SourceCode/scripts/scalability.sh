#!/bin/bash
# 构建缺乏1% 2% ～ 32%顶点的Part_Build索引

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
ln -sf ../shells/SEA_test.jar ./

EXP_FOLDER_SUFFIX=$4

root="${CURRENT_DIR}/../exp_data/"
############ BUILD ################
IDX_ALGO=$2
if [ ${IDX_ALGO} == "rKACS" ]; then
  IDX_ALGO=ATC
  fi
BUILD_THREAD_NUM=6
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

############ Prune ##################
Prune_Root=${root}/Prune/${DATASET}

case $3 in
  build)
    check_dir_and_make_if_absent_notquit ${INDEX_PREFIX}/log
    echo "Building ..."
    echo "./Build ${DATASET} ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${DEGREE} ${BUILD_L} \
                 ${ALPHA} ${BETA} ${RPPP} ${BUILD_THREAD_NUM} ${COMBASE} ${SMETHOD} ${METRIC}"
    time ./Build ${DATASET} ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${DEGREE} ${BUILD_L} \
       ${ALPHA} ${BETA} ${RPPP} ${BUILD_THREAD_NUM} ${COMBASE} ${SMETHOD} ${METRIC} |& tee ${INDEX_LOG_PATH} 2>&1
   ;;
esac

############ PART BUILD ################
for VALID_NODE_PERCENT in 32
#for VALID_NODE_PERCENT in 1 2 4 8 16 32
do

echo "=============iter begin=============="
BUILD_THREAD_NUM=6
#VALID_NODE_PERCENT=5
PART_INDEX_PREFIX="${root}/part_indices/${DATASET}/base/"
PART_INDEX_PATH=${PART_INDEX_PREFIX}"/${DATASET}_${IDX_ALGO}_L${BUILD_L}_R${DEGREE}_alp${ALPHA}_beta${BETA}_del${VALID_NODE_PERCENT}.chci"
PART_INDEX_LOG_PATH="${PART_INDEX_PREFIX}/log/${DATASET}_L${BUILD_L}_R${DEGREE}_alp${ALPHA}_beta${BETA}_del${VALID_NODE_PERCENT}.chci"
VALID_NODE_PATH=${CURRENT_DIR}/../datasets/Prune/${DATASET}/${DATASET}_remain_${VALID_NODE_PERCENT}%.txt

ADD_START_PERCENT=${VALID_NODE_PERCENT}
ADD_START=${PART_INDEX_PATH}
ADD_NODES_PATH_MULTI=""
ADD_INDEX_LOG_PATH_MULTI=""

ADD_NODES_PATH=${CURRENT_DIR}/../datasets/Prune/${DATASET}/${DATASET}_del_${ADD_START_PERCENT}%.txt
ADD_INDEX_PREFIX=${root}/part_indices/${DATASET}/Add/
ADD_INDEX_LOG_PREFIX=${ADD_INDEX_PREFIX}/log/
ADD_INDEX_PATH=${ADD_INDEX_PREFIX}/${DATASET}_${IDX_ALGO}_add_${ADD_START_PERCENT}%.chci
ADD_INDEX_LOG_PATH=${ADD_INDEX_LOG_PREFIX}/${DATASET}_${IDX_ALGO}_add_${ADD_START_PERCENT}%.log

ADD_NODES_PATH_MULTI=${ADD_NODES_PATH},${ADD_NODES_PATH_MULTI}
ADD_INDEX_PATH_MULTI=${ADD_INDEX_PATH},${ADD_INDEX_PATH_MULTI}

echo $ADD_INDEX_PATH_MULTI
echo $ADD_NODES_PATH_MULTI

DEL_NODES_PATH=${ADD_NODES_PATH}

############### Retrieve Param ######################
DEFAULT_THREAD_NUM=60
RETRIEVE_RES_PREFIX="${root}/retrieve/sup_exp2/${EXP_FOLDER_SUFFIX}/${DATASET}/"
RETRIEVE_RES_PATH="${RETRIEVE_RES_PREFIX}${2}_$4_r${r}_layer${LAYER}_add_${ADD_START_PERCENT}.log"
RETRIEVE_RES_LOG_PATH="${RETRIEVE_RES_PREFIX}log/${2}_$4_r${r}_layer${LAYER}_add_${ADD_START_PERCENT}.log"

############ VAC ###################
VAC_PREFIX="${root}vac/${EXP_FOLDER_SUFFIX}/${DATASET}/"
VAC_RES_PATH="${VAC_PREFIX}with_r${r}_layer${LAYER}_add${VALID_NODE_PERCENT}.log"
VAC_RES_PATH_WO="${VAC_PREFIX}wo_r${r}_layer${LAYER}.log"
VAC_RUN_WITH=1
VAC_RUN_WO=0

############ SEA ###################
SEA_PREFIX="${root}sea/${EXP_FOLDER_SUFFIX}/${DATASET}/"
SEA_RES_PATH_WITH="${SEA_PREFIX}with_r${r}_layer${LAYER}_add${VALID_NODE_PERCENT}.log"
SEA_RES_PATH_WO="${SEA_PREFIX}wo_r${r}_layer${LAYER}.log"
SEA_RUN_WO=0

############ ATC ###################
ATC_d=3
ATC_gamma=0.50
ATC_RUN_WITH=1
ATC_RUN_WO=0
ATC_PREFIX="${root}atc/${EXP_FOLDER_SUFFIX}/${DATASET}/"
ATC_RES_PATH="${ATC_PREFIX}with_r${r}_layer${LAYER}_add_${VALID_NODE_PERCENT}.log"
ATC_RES_PATH_WO=${root}atc/${DATASET}/WO/wo_atcgamma_${ATC_gamma}_atcd_${ATC_d}.log

rKACS_RUN_WO=0
case $4 in
  Add)
    INTEGRATE_INDEX_PATH=${BASE_INDEX_PATH}
    INDEX_PATH=${ADD_INDEX_PATH}
    ############### Retrieve Param ######################
    RETRIEVE_RES_PREFIX="${root}/retrieve/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    RETRIEVE_RES_PATH="${RETRIEVE_RES_PREFIX}${2}_$4_r${r}_layer${LAYER}_add_${ADD_START_PERCENT}.log"
    RETRIEVE_RES_LOG_PATH="${RETRIEVE_RES_PREFIX}log/${2}_$4_r${r}_layer${LAYER}_add_${ADD_START_PERCENT}.log"
    EXP_VALID_NODE_PATH=null

    ############ VAC ###################
    if [ ${VALID_NODE_PERCENT} != 1 ]; then
        VAC_RUN_WO=0
    fi
    VAC_PREFIX="${root}vac/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    VAC_RES_PATH="${VAC_PREFIX}with_r${r}_layer${LAYER}_add${VALID_NODE_PERCENT}.log"
    VAC_RES_PATH_WO="${VAC_PREFIX}wo_r${r}_layer${LAYER}.log"

    ############ SEA ###################
    SEA_PREFIX="${root}sea/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    SEA_RES_PATH_WITH="${SEA_PREFIX}with_r${r}_layer${LAYER}_add${VALID_NODE_PERCENT}.log"
    SEA_RES_PATH_WO="${SEA_PREFIX}wo_r${r}_layer${LAYER}.log"

    ############ ATC ###################
    if [ ${VALID_NODE_PERCENT} != 1 ]; then
        ATC_RUN_WO=0
    fi
    ATC_PREFIX="${root}atc/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    ATC_RES_PATH="${ATC_PREFIX}with_r${r}_layer${LAYER}_add_${VALID_NODE_PERCENT}.log"
    ATC_RES_PATH_WO=${ATC_PREFIX}/wo_atcgamma_${ATC_gamma}_atcd_${ATC_d}_origin.log

    ############ rKACS ##################
    rKACS_PREFIX="${root}rKACS/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    rKACS_RES_PATH_WITH="${rKACS_PREFIX}with_r${r}_layer${LAYER}_dschange${VALID_NODE_PERCENT}.log"
    rKACS_RES_PATH_WO="${rKACS_PREFIX}wo_layer${LAYER}_dschange${VALID_NODE_PERCENT}.log"
    rKACS_FILE_PREFIX="${root}/..//datasets/rKACS/${DATASET}/"
    rKACS_TRUSSNESS_FILE=${rKACS_FILE_PREFIX}${DATASET}_trussness_origin.txt
    rKACS_SIMILARITY_FILE=${rKACS_FILE_PREFIX}${DATASET}_similarity_origin.txt
    rKACS_RUN_WO=0
    if [ ${VALID_NODE_PERCENT} != 1 ]; then
        rKACS_RUN_WO=0
    fi
    ;;
  Del)

    ############### Retrieve Param ######################
    RETRIEVE_RES_PREFIX="${root}/retrieve/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    RETRIEVE_RES_PATH="${RETRIEVE_RES_PREFIX}${2}_$4_r${r}_layer${LAYER}_del_${VALID_NODE_PERCENT}.log"
    RETRIEVE_RES_LOG_PATH="${RETRIEVE_RES_PREFIX}log/${2}_$4_r${r}_layer${LAYER}_del_${VALID_NODE_PERCENT}.log"
    EXP_VALID_NODE_PATH=${VALID_NODE_PATH}

    ############ VAC ###################
    VAC_RES_PATH="${VAC_PREFIX}with_r${r}_layer${LAYER}_del${VALID_NODE_PERCENT}.log"
    VAC_RES_PATH_WO="${VAC_PREFIX}wo_layer${LAYER}_del${VALID_NODE_PERCENT}.log"
    ############ SEA ###################
    SEA_RES_PATH_WITH="${SEA_PREFIX}with_r${r}_layer${LAYER}_del${VALID_NODE_PERCENT}.log"
    SEA_RES_PATH_WO="${SEA_PREFIX}wo_layer${LAYER}_del${VALID_NODE_PERCENT}.log"
    ############ ATC ###################
    ATC_PREFIX="${root}atc/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    ATC_RES_PATH="${ATC_PREFIX}with_r${r}_layer${LAYER}_del_${VALID_NODE_PERCENT}.log"
    ATC_RES_PATH_WO=${ATC_PREFIX}/wo_atcgamma_${ATC_gamma}_atcd_${ATC_d}_del${VALID_NODE_PERCENT}.log
    ############ rKACS ##################
    rKACS_PREFIX="${root}rKACS/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    rKACS_RES_PATH_WITH="${rKACS_PREFIX}with_r${r}_layer${LAYER}_dschange${VALID_NODE_PERCENT}.log"
    rKACS_RES_PATH_WO="${rKACS_PREFIX}wo_layer${LAYER}_dschange${VALID_NODE_PERCENT}.log"
    rKACS_FILE_PREFIX="${root}/..//datasets/rKACS/${DATASET}/"
    rKACS_TRUSSNESS_FILE=${rKACS_FILE_PREFIX}${DATASET}_trussness_del${VALID_NODE_PERCENT}.txt
    rKACS_SIMILARITY_FILE=${rKACS_FILE_PREFIX}${DATASET}_similarity_del${VALID_NODE_PERCENT}.txt
    rKACS_RUN_WO=0
    INTEGRATE_INDEX_PATH=${PART_INDEX_PATH}
        ;;

  Base_Add)
    INTEGRATE_INDEX_PATH=${BASE_INDEX_PATH}
    INDEX_PATH=${BASE_INDEX_PATH}
    ############### Retrieve Param ######################
    RETRIEVE_RES_PREFIX="${root}/retrieve/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    RETRIEVE_RES_PATH="${RETRIEVE_RES_PREFIX}${2}_$4_r${r}_layer${LAYER}.log"
    RETRIEVE_RES_LOG_PATH="${RETRIEVE_RES_PREFIX}log/${2}_$4_r${r}_layer${LAYER}.log"
    EXP_VALID_NODE_PATH=null

    ############ VAC ###################
    if [ ${VALID_NODE_PERCENT} != 1 ]; then
        VAC_RUN_WO=0
    fi
    VAC_PREFIX="${root}vac/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    VAC_RES_PATH="${VAC_PREFIX}with_r${r}_layer${LAYER}.log"
    VAC_RES_PATH_WO="${VAC_PREFIX}wo_r${r}_layer${LAYER}.log"

    ############ SEA ###################
    SEA_PREFIX="${root}sea/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    SEA_RES_PATH_WITH="${SEA_PREFIX}with_r${r}_layer${LAYER}.log"
    SEA_RES_PATH_WO="${SEA_PREFIX}wo_r${r}_layer${LAYER}.log"

    ############ ATC ###################
    if [ ${VALID_NODE_PERCENT} != 1 ]; then
        ATC_RUN_WO=0
    fi
    ATC_PREFIX="${root}atc/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    ATC_RES_PATH="${ATC_PREFIX}with_r${r}_layer${LAYER}.log"
    ATC_RES_PATH_WO=${ATC_PREFIX}/wo_atcgamma_${ATC_gamma}_atcd_${ATC_d}_origin.log

    ############ rKACS ##################
    rKACS_PREFIX="${root}rKACS/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    rKACS_RES_PATH_WITH="${rKACS_PREFIX}with_r${r}_layer${LAYER}.log"
    rKACS_RES_PATH_WO="${rKACS_PREFIX}wo_layer${LAYER}.log"
    rKACS_FILE_PREFIX="${root}/..//datasets/rKACS/${DATASET}/"
    rKACS_TRUSSNESS_FILE=${rKACS_FILE_PREFIX}${DATASET}_trussness_origin.txt
    rKACS_SIMILARITY_FILE=${rKACS_FILE_PREFIX}${DATASET}_similarity_origin.txt
    rKACS_RUN_WO=0
    if [ ${VALID_NODE_PERCENT} != 1 ]; then
        rKACS_RUN_WO=0
    fi
    ;;
  Base_Del)

    ############### Retrieve Param ######################
    RETRIEVE_RES_PREFIX="${root}/retrieve/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    RETRIEVE_RES_PATH="${RETRIEVE_RES_PREFIX}${2}_$4_r${r}_layer${LAYER}_del_${VALID_NODE_PERCENT}.log"
    RETRIEVE_RES_LOG_PATH="${RETRIEVE_RES_PREFIX}log/${2}_$4_r${r}_layer${LAYER}_del_${VALID_NODE_PERCENT}.log"
    EXP_VALID_NODE_PATH=${VALID_NODE_PATH}

    ############ VAC ###################
    VAC_RES_PATH="${VAC_PREFIX}with_r${r}_layer${LAYER}_del${VALID_NODE_PERCENT}.log"
    VAC_RES_PATH_WO="${VAC_PREFIX}wo_layer${LAYER}_del${VALID_NODE_PERCENT}.log"
    ############ SEA ###################
    SEA_RES_PATH_WITH="${SEA_PREFIX}with_r${r}_layer${LAYER}_del${VALID_NODE_PERCENT}.log"
    SEA_RES_PATH_WO="${SEA_PREFIX}wo_layer${LAYER}_del${VALID_NODE_PERCENT}.log"
    ############ ATC ###################
    ATC_PREFIX="${root}atc/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    ATC_RES_PATH="${ATC_PREFIX}with_r${r}_layer${LAYER}_del_${VALID_NODE_PERCENT}.log"
    ATC_RES_PATH_WO=${ATC_PREFIX}/wo_atcgamma_${ATC_gamma}_atcd_${ATC_d}_del${VALID_NODE_PERCENT}.log
    ############ rKACS ##################
    rKACS_PREFIX="${root}rKACS/${EXP_FOLDER_SUFFIX}/${DATASET}/"
    rKACS_RES_PATH_WITH="${rKACS_PREFIX}with_r${r}_layer${LAYER}_dschange${VALID_NODE_PERCENT}.log"
    rKACS_RES_PATH_WO="${rKACS_PREFIX}wo_layer${LAYER}_dschange${VALID_NODE_PERCENT}.log"
    rKACS_FILE_PREFIX="${root}/..//datasets/rKACS/${DATASET}/"
    rKACS_TRUSSNESS_FILE=${rKACS_FILE_PREFIX}${DATASET}_trussness_del${VALID_NODE_PERCENT}.txt
    rKACS_SIMILARITY_FILE=${rKACS_FILE_PREFIX}${DATASET}_similarity_del${VALID_NODE_PERCENT}.txt

    INDEX_PATH=${PART_INDEX_PATH}
    INTEGRATE_INDEX_PATH=${PART_INDEX_PATH}
    ;;
esac
set -x
case $3 in
retrieve)
    check_dir_and_make_if_absent_notquit ${RETRIEVE_RES_PREFIX}/log
  case $4 in
  Add)
    ./Retrieve ${DATASET}  ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${RETRIEVE_RES_PATH} ${RETRIEVE_RES_LOG_PATH} ${QUERY_PATH} \
             ${r} ${LAYER} ${COMBASE} ${SMETHOD} ${METRIC} ${DEFAULT_THREAD_NUM} ${rKACS_out}
    ;;
  Del)
    ./Retrieve_Del ${DATASET}  ${BASE_INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${RETRIEVE_RES_PATH} ${RETRIEVE_RES_LOG_PATH} ${QUERY_PATH} ${DEL_NODES_PATH} \
                 ${r} ${LAYER} ${COMBASE} ${SMETHOD} ${METRIC} ${DEFAULT_THREAD_NUM} ${rKACS_out}
   ;;
  Base_Add)
  ./Retrieve ${DATASET}  ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${RETRIEVE_RES_PATH} ${RETRIEVE_RES_LOG_PATH} ${QUERY_PATH} \
            ${r} ${LAYER} ${COMBASE} ${SMETHOD} ${METRIC} ${DEFAULT_THREAD_NUM} ${rKACS_out}
  ;;
  Base_Del)
  ./Retrieve ${DATASET}  ${INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${RETRIEVE_RES_PATH} ${RETRIEVE_RES_LOG_PATH} ${QUERY_PATH} \
            ${r} ${LAYER} ${COMBASE} ${SMETHOD} ${METRIC} ${DEFAULT_THREAD_NUM} ${rKACS_out}
  ;;
  esac
  ;;
run)
  case $2 in
VAC)
    check_dir_and_make_if_absent_notquit ${VAC_PREFIX}
  echo "./VAC_test ${DATASET}  ${BASE_INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${EXP_VALID_NODE_PATH} ${RETRIEVE_RES_PATH} ${VAC_RES_PATH} ${VAC_RES_PATH_WO}\
                ${LAYER} ${VAC_RUN_WITH} ${VAC_RUN_WO} 1 "
  ./VAC_test ${DATASET}  ${INTEGRATE_INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${RETRIEVE_RES_PATH} ${VAC_RES_PATH} ${VAC_RES_PATH_WO} \
              ${LAYER} ${VAC_RUN_WITH} ${VAC_RUN_WO} 1
  ;;
ATC)
    check_dir_and_make_if_absent_notquit ${ATC_PREFIX}
  echo "./ATC_test ${DATASET}  ${BASE_INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${EXP_VALID_NODE_PATH} ${RETRIEVE_RES_PATH} ${ATC_RES_PATH} ${ATC_RES_PATH_WO}\
              ${LAYER} ${ATC_d} ${ATC_gamma} ${ATC_RUN_WITH} ${ATC_RUN_WO} ${DEFAULT_THREAD_NUM}"
  ./ATC_test ${DATASET}  ${INTEGRATE_INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${RETRIEVE_RES_PATH} ${ATC_RES_PATH} ${ATC_RES_PATH_WO}\
              ${LAYER} ${ATC_d} ${ATC_gamma} ${ATC_RUN_WITH} ${ATC_RUN_WO} ${DEFAULT_THREAD_NUM}
  ;;
SEA)
    check_dir_and_make_if_absent_notquit ${SEA_PREFIX}
  echo "./SEA_test.jar k-core ${DATASET} ${DATASET_TYPE} ${RETRIEVE_RES_PATH} ${LAYER} ${META_PATH} ${SEA_RUN_WO} ${SEA_RES_PATH_WITH} ${SEA_RES_PATH_WO} \
                ${graph} ${vertex} ${edge} ${data}"
  java -jar ./SEA_test.jar k-core ${DATASET} ${DATASET_TYPE} ${RETRIEVE_RES_PATH} ${LAYER} ${META_PATH} ${SEA_RUN_WO} ${SEA_RES_PATH_WITH} ${SEA_RES_PATH_WO} \
              ${graph} ${vertex} ${edge} ${data} ${VALID_NODE_PATH}
              ;;
rKACS)
    check_dir_and_make_if_absent_notquit ${rKACS_PREFIX}
    check_dir_and_make_if_absent_notquit ${rKACS_FILE_PREFIX}
    ./rKACS_Query ${DATASET} ${graph} ${vertex} ${edge} ${data} ${VALID_NODE_PATH} ${rKACS_TRUSSNESS_FILE} ${rKACS_SIMILARITY_FILE} ${LAYER} 1 ${RETRIEVE_RES_PATH} ${rKACS_RES_PATH_WITH} ${rKACS_RES_PATH_WO} ${rKACS_RUN_WO}
    ;;
  esac
  ;;

PartBuild)
    check_dir_and_make_if_absent_notquit ${PART_INDEX_PREFIX}/log
    echo "Building ..."
     time ./Part_Build ${DATASET} ${PART_INDEX_PATH} ${graph}  ${vertex} ${edge} ${data} ${VALID_NODE_PATH} ${DEGREE} ${BUILD_L} \
                      ${ALPHA} ${BETA} ${RPPP} ${BUILD_THREAD_NUM} ${COMBASE} ${SMETHOD} ${METRIC} |& tee ${PART_INDEX_LOG_PATH} 2>&1
   ;;
AddNodes)
    check_dir_and_make_if_absent_notquit ${ADD_INDEX_LOG_PREFIX}
    echo "Adding ..."
   ./Add_Nodes ${DATASET} ${ADD_START} ${graph}  ${vertex} ${edge} ${data} ${VALID_NODE_PATH} ${ADD_NODES_PATH} ${ADD_INDEX_PATH} ${DEGREE} ${BUILD_L} \
                    ${ALPHA} ${BETA} ${RPPP} ${BUILD_THREAD_NUM} ${COMBASE} ${SMETHOD} ${METRIC}
   ;;

esac
set +x
done
