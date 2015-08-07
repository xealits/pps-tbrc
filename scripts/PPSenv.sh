#!/bin/sh

function ppsRun() {
  cd $PPS_PATH;
  $PPS_PATH"/ppsRun";
}

function ppsGUI() {
  cd $PPS_PATH;
  /usr/bin/python $PPS_PATH/../scripts/RunControl.py
}

function ppsDQM() {
  declare -a proc=("gastof" "quartic" "daq" "db")
  function start_proc() {
    cd $PPS_PATH;
    $PPS_PATH/dqm/"$1" > /dev/null 2>&1 &
  }
  function stop_proc() {
    killall -9 "$1";
  }
  function status_proc() {
    state=`ps -C "$1" -o stat | tail -1`
    time_started=`ps -C "$1" -o lstart | tail -1`
    printf "%15s: %2s\t%-10s\n" "$1" "$state" "$time_started";
  }
  if [ -z "$1" ] ; then
    echo "Usage: ppsDQM [start|stop|status]"
    return
  fi
  cd $PPS_PATH;
  if [ "$1" == "start" ] ; then
    echo "Starting DQM processes..."
    for i in "${proc[@]}" ; do
       start_proc $i;
    done
  elif [ "$1" == "stop" ] ; then
    echo "Stopping DQM processes..."
    for i in "${proc[@]}" ; do
       stop_proc $i;
    done
  elif [ "$1" == "status" ] ; then
    echo "List of DQM threads:";
    for i in "${proc[@]}" ; do
       status_proc $i;
    done
  fi
}
