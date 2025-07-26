#!/usr/sbin/sh
#
#
#
# Copyright (c) 2025, Gold Of Leprechauns and/or its affiliates. All rights reserved.
#

# This service is managed by smf(5).  Thus, this script provides
#

state="$1"

name="acu"
acu_name="$name"
acu_config="/usr/local/etc/$acu_name/config.json"
acu_pid="/var/run/$acu_name.pid"

# Command start ACU
command="/usr/local/bin/acu-server"

case "$state" in
'status')
        echo 'Status process ACU'
        pid=`cat $acu_pid 2>/dev/null`
		if [ "$pid" != "" ] && kill -s 0 $pid 2>/dev/null; then
			echo "$acu_name is running as pid $pid."
		else
			echo "$acu_name is not running."
		fi
        ;;

'start')
        echo 'Starting process ACU'
        pid=`cat $acu_pid 2>/dev/null`
		if [ "$pid" != "" ] && kill -s 0 $pid 2>/dev/null; then
			echo "$acu_name is already running."
		else
			rm -f $acu_pid 2>/dev/null
			$command --config=$acu_config --log="/var/log/$acu_name.log" --logLevel=6 &
			# $command --config=$acu_config --log="/var/log/$acu_name.log" --logLevel=6 2>/dev/null
			sleep 1
			pid=`cat $acu_pid 2>/dev/null`
			if [ "$pid" != "" ]; then
				echo "OK: $acu_name"
			else
				echo "FAILED: $acu_name; see logs"
			fi
		fi
        ;;

'stop')
        echo 'Stopping process ACU'
        pid=`cat $acu_pid 2>/dev/null`
		if [ "$pid" = "" ]; then
			echo "$acu_name is not running (pid file is empty)."
		else
			kill $pid
			rm -f $acu_pid 2>/dev/null
			echo "OK: $acu_name"
		fi
        ;;

*)
        echo "Usage: $0 { start | stop | status }"
        exit 1
        ;;
esac
exit 0
