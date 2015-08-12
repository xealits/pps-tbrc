#!/bin/sh

if [ -z "$1" ] ; then
  echo "Usage: "$0" <run_id> <board_id>"
  exit
fi
if [ -z "$2" ] ; then
  echo "Usage: "$0" <run_id> <board_id>"
  exit
fi

python $PPS_BASE_PATH/scripts/run_over_all_files.py $1 $2 && hadd -f "run"$1"_board"$2".root" $PPS_DATA_PATH/events_$1_*_board$2.root 

