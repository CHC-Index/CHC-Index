#!/bin/sh

#root=../datasets/
root="$(pwd)/../datasets/"
DBLP() {
   DBLP_root=${root}"DBLP/";
   graph="${DBLP_root}graph.txt";
   vertex="${DBLP_root}vertex.txt";
   edge="${DBLP_root}edge.txt";
   data="${DBLP_root}DBLPdata.txt";
  DATASET=DBLP
  ALPHA=1.2
  DEGREE=24
  BUILD_L=40
  BETA=0.80
  RPPP=1
  BUILD_THREAD_NUM=6
  DATASET_TYPE=1
  META_PATH="1,0,1;3,0"
  SMETHOD=opt2
  ATC_r=0.40
    VAC_r=0.05
    SEA_r=0.05
    rKACS_r=0.60
    SEA_DATASET=dblp
    ATC_perc=0.50
}


IMDB(){
  IMDB_root="${root}IMDB/";
   graph="${IMDB_root}IMDBgraph.txt";
   vertex="${IMDB_root}oldIMDBvertex.txt";
   edge="${IMDB_root}IMDBedge.txt";
   data="${IMDB_root}IMDBpersonattributes.txt";
    DATASET=IMDB_Person
    BUILD_L=48
    ALPHA=1.2
    DEGREE=26
  RPPP=1
  BETA=0.80
  DATASET_TYPE=1
    BUILD_THREAD_NUM=16
  META_PATH="1,0,1;21,9"
  SMETHOD=opt2
   ATC_r=0.30
      VAC_r=0.20
      SEA_r=0.80
      rKACS_r=0.70
    ATC_perc=0.60

}

IMDB_Person(){
  IMDB_root="${root}IMDB/";
   graph="${IMDB_root}IMDBgraph.txt";
   vertex="${IMDB_root}oldIMDBvertex.txt";
   edge="${IMDB_root}IMDBedge.txt";
   data="${IMDB_root}IMDBpersonattributes.txt";
    DATASET=IMDB_Person
    BUILD_L=48
    ALPHA=1.2
    DEGREE=26
  RPPP=1
  BETA=0.80
  DATASET_TYPE=1
    BUILD_THREAD_NUM=16
  META_PATH="1,0,1;21,9"
  SMETHOD=opt2
   ATC_r=0.30
      VAC_r=0.20
      SEA_r=0.80
      rKACS_r=0.70
    ATC_perc=0.60

}

IMDB_Movie(){
  IMDB_root="${root}IMDB/";
  graph="${IMDB_root}IMDBgraph.txt";
   vertex="${IMDB_root}oldIMDBvertex.txt";
   edge="${IMDB_root}IMDBedge.txt";
   data="${IMDB_root}IMDBmovieattributes.txt";
      DATASET=IMDB_Movie
      BUILD_L=48
      ALPHA=1.2
      DEGREE=26
  RPPP=1
  BETA=0.80
  DATASET_TYPE=1
      BUILD_THREAD_NUM=16
  META_PATH="0,1,0,1,0;3,12,0,15"
  SMETHOD=opt2
  ATC_r=0.30
    VAC_r=0.20
    SEA_r=0.30
    rKACS_r=0.50
    ATC_perc=0.40

}

Twitch(){
   Twitch_root="${root}Twitch/";
   graph="${Twitch_root}oldtwitch_edges.txt";
   vertex="null";
   edge="${Twitch_root}oldtwitch_edges.txt";
   # edge="null";
   data="${Twitch_root}twitch_attributes.txt";
  DATASET=Twitch
          BUILD_L=56
          ALPHA=1.2
          DEGREE=24
  RPPP=1
          BUILD_THREAD_NUM=16
  DATASET_TYPE=0
  BETA=0.80
  META_PATH="0,0;0"
  SMETHOD=opt2
  ATC_r=0.60
    VAC_r=0.05
    SEA_r=0.05
    rKACS_r=0.30
    ATC_perc=0.60

}

LiveJournal(){
  LiveJournal_root="${root}LiveJournal/"
               graph="${LiveJournal_root}livejournal_newedges.txt";
               vertex="null";
               edge="${LiveJournal_root}livejournal_newedges.txt";
               data="${LiveJournal_root}livejournalattributes.txt";
 DATASET=LiveJournal
         BUILD_L=80
         ALPHA=1.2
         DEGREE=28
  RPPP=1
  BETA=0.80
  DATASET_TYPE=0
         BUILD_THREAD_NUM=16
  META_PATH="0,0;0"
  SMETHOD=opt2
  ATC_r=0.10
    VAC_r=0.05
    SEA_r=0.05
    rKACS_r=0.30
    ATC_perc=0.40
}

Facebook(){
  Facebook_root="${root}Facebook/";
#  graph="null";
   graph="${Facebook_root}0_edges.txt";
  vertex="null";
   edge="${Facebook_root}0_edges.txt";
  data="${Facebook_root}fb_merge.featorigin";
  DATASET=Facebook
         BUILD_L=16
         ALPHA=1.2
         DEGREE=8
  RPPP=1
  BETA=0.80
  DATASET_TYPE=0
         BUILD_THREAD_NUM=16
  META_PATH="0,0;0"
  SMETHOD=opt2
}
FB0(){
  Facebook_root="${root}Facebook/";
   graph="null";
   vertex="null";
   edge="${Facebook_root}0_edges.txt";
  data="${Facebook_root}fb_merge.featorigin";
  DATASET=FB0
          BUILD_L=16
          ALPHA=1.2
          DEGREE=8
  RPPP=1
  BETA=0.80
  DATASET_TYPE=0
  BUILD_THREAD_NUM=16
  META_PATH="0,0;0"
  SMETHOD=opt2
}


check_dir_and_make_if_absent_notquit() {
  local dir=$1
  if [ -d $dir ]; then
    echo ""
  else
    mkdir -p ${dir}
  fi
}

check_dir_and_make_if_absent() {
  local dir=$1
  if [ -d $dir ]; then
    echo "Directory $dir is already exit. Remove or rename it and then re-run."
    exit 1
  else
    mkdir -p ${dir}
  fi
}
