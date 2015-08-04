#!/bin/sh

function ppsRun() {
  cd $PPS_PATH;
  $PPS_PATH"/ppsRun";
}

function ppsDQM() {
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
    start_proc gastof;
    start_proc quartic;
    start_proc daq;
  elif [ "$1" == "stop" ] ; then
    echo "Stopping DQM processes..."
    stop_proc gastof;
    stop_proc quartic;
    stop_proc daq;
  elif [ "$1" == "status" ] ; then
    echo "List of DQM threads:";
    status_proc gastof;
    status_proc quartic;
    status_proc daq;
  fi
}
