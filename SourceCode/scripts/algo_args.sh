#!/bin/sh

ATC() {
 PERCENTAGE=${ATC_r}
   COMBASE=truss
   METRIC=TEXT
   r=${ATC_r}
   RUN_ALGO="./ATC_test"
}
VAC(){
 PERCENTAGE=${VAC_r}
  COMBASE=truss
  METRIC=HYB
  r=${VAC_r}
  RUN_ALGO="./VAC_test"
}
rKACS(){
  PERCENTAGE=${rKACS_r}
 COMBASE=truss
   METRIC=TEXT
   r=${rKACS_r}
   RUN_ALGO="./rKACS_Query"
}
SEA(){
  PERCENTAGE=${SEA_r}
 COMBASE=core
 METRIC=HYB
 r=${SEA_r}
 RUN_ALGO="jave -jar ./SEA_test.jar"
}