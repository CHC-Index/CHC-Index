#!/bin/bash

source ./dataset_args.sh
# DBLP
#IMDB_Person
# IMDB_Movie
#FourSquare
#CORA
# Twitch
#LiveJournal
#Facebook
#DBp_Soccer
#Github

EXP_ROOT=${HOME}
if [ ${HOME} == "/home/zjlab" ]
then
  EXP_ROOT="${HOME}/pzy"
fi

if [ ${HOME} == "/home/gy" ]
then
  EXP_ROOT="${HOME}"
fi

if [ ${HOME} == "/root" ]
then
  EXP_ROOT="/home/gy"
fi

PZY_ROOT="/home/pzy"
EXP_ROOT="${EXP_ROOT}/CNNS-Experiment"
COMBASE=core
SMETHOD=A
PAPER_B=0.8
METRIC=SEA
QUERY_PREFIX_PATH="${EXP_ROOT}/query/${DATASET}"

############# Build Info  #############
INDEX_ROOT=${EXP_ROOT}/Index
if [ ${HOME} == "/home/gy" ]
then
  INDEX_ROOT="${PZY_ROOT}/CNNS-Experiment/Index"
  QUERY_PREFIX_PATH="${PZY_ROOT}/CNNS-Experiment/query/${DATASET}"
fi

if [ ${HOME} == "/root" ]
then
  INDEX_ROOT="${PZY_ROOT}/CNNS-Experiment/new_Index"
  QUERY_PREFIX_PATH="${EXP_ROOT}/CNNS-Experiment/query/${DATASET}"
fi

INDEX_PREFIX_PATH=${INDEX_ROOT}/${DATASET}
INDEX_PATH=${INDEX_PREFIX_PATH}/${DATASET}_smethod${SMETHOD}_based${COMBASE}_alpha${ALPHA}_beta${PAPER_B}_deg${DEGREE}_buildL${BUILD_L}_metric${METRIC}.txt
BUILD_LOG_ROOT=${INDEX_PREFIX_PATH}/log
BUILD_LOG_PATH=${BUILD_LOG_ROOT}/${DATASET}_smethod${SMETHOD}_based${COMBASE}_alpha${ALPHA}_beta${PAPER_B}_deg${DEGREE}_buildL${BUILD_L}_metric${METRIC}.log

#############  Query Info #############
CORENUM=15
QUERYNUM=150
QUERY_PATH="${QUERY_PREFIX_PATH}/${DATASET}_${QUERYNUM}queries_${CORENUM}${COMBASE}"
TOPN=5000
PERCENTAGE=0.60
RANGE=0.10
SEARCH_THREADS=8
COMPONENT=0

queryRes_Root="${EXP_ROOT}/ExperimentResult/CNNS-Search-Result"
queryRes_TopN_Path=${queryRes_Root}/${DATASET}_coreness${CORENUM}_topN${TOPN}_methodA_based${COMBASE}_formalTest_NORMAL.txt
queryLog_TopN_Path=${queryRes_Root}/log/${DATASET}_coreness${CORENUM}_topN${TOPN}_methodA_based${COMBASE}_formalTest_NORMAL.txt

queryRes_Percent_Path=${queryRes_Root}/${DATASET}_coreness${CORENUM}_percent${PERCENTAGE}_methodA_based${COMBASE}_formalTest_NORMAL.txt
queryLog_Percent_Path=${queryRes_Root}/log/${DATASET}_coreness${CORENUM}_percent${PERCENTAGE}_methodA_based${COMBASE}_formalTest_NORMAL.txt

queryRes_RS_Path=${queryRes_Root}/${DATASET}_coreness${CORENUM}_Range${RANGE}_methodA_based${COMBASE}_formalTest_NORMAL.txt
queryRes_RS_Path_rKACS=${queryRes_Root}/${DATASET}_coreness${CORENUM}_Range${RANGE}_methodA_based${COMBASE}_formalTest_NORMAL.txtrKACS
queryLog_RS_Path=${queryRes_Root}/log/${DATASET}_coreness${CORENUM}_Range${RANGE}_methodA_based${COMBASE}_formalTest_NORMAL.txt

#############  VAC Info  #############
run_wo=0
VACqueryRes_Root=${EXP_ROOT}/ExperimentResult/VAC
VACqueryRes_TopN_Path=${VACqueryRes_Root}/${DATASET}_coreness${CORENUM}_topN${TOPN}_methodA_based${COMBASE}_formalTest_VACSearchRes_Normal.txt

VACqueryRes_Percent_Path=${VACqueryRes_Root}/${DATASET}_coreness${CORENUM}_percent${PERCENTAGE}_methodA_based${COMBASE}_formalTest_VACSearchRes_Normal.txt
VACqueryRes_RS_Path=${VACqueryRes_Root}/${DATASET}_coreness${CORENUM}_Range${RANGE}_methodA_based${COMBASE}_formalTest_VACSearchRes.txt


