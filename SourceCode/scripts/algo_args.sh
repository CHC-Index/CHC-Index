#!/bin/sh

ATC() {
 PERCENTAGE=${ATC_BEST}
   COMBASE=truss
   METRIC=TEXT
   r=${ATC_BEST}
   RUN_ALGO="./ATC_test"
}
VAC(){
 PERCENTAGE=${VAC_BEST}
  COMBASE=truss
  METRIC=HYB
  r=${VAC_BEST}
  RUN_ALGO="./VAC_test"
}
rKACS(){
  PERCENTAGE=${rKACS_BEST}
 COMBASE=truss
   METRIC=TEXT
   r=${rKACS_BEST}
   RUN_ALGO="./rKACS_Query"
}
SEA(){
  PERCENTAGE=${SEA_BEST}
 COMBASE=core
 METRIC=HYB
 r=${SEA_BEST}
 RUN_ALGO="jave -jar ./SEA_test.jar"
}