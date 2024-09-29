#!/bin/sh

# PROVIDE: acu
# BEFORE:  LOGIN cleanvar
# KEYWORD: shutdown

. /etc/rc.subr

name="acu"
rcvar=acu_enable

load_rc_config $name

# Logging options. By default syslog is used, it allows easy log rotation.
logpri='local3.info'

# Varriables
: ${acu_enable="NO"}
: ${acu_name="acu"}
: ${acu_config="/usr/local/etc/$acu_name/config.json"}
: ${acu_pid="/var/run/$acu_name.pid"}

# Commands
start_cmd="start"
restart_cmd="restart"
stop_cmd="stop"

# Check enable daemon
eval "${rcvar}=\${${rcvar}:-'NO'}"

# Command start ACU
command="/usr/local/bin/acu-server"

# Start command
start(){
	pid=`cat $acu_pid 2>/dev/null`
	if [ "$pid" != "" ] && kill -s 0 $pid 2>/dev/null; then
		echo "$acu_name is already running"
	else
		rm -f $acu_pid 2>/dev/null
		$command --config=$acu_config --log="/var/log/$acu_name.log" --logLevel=6 2>/dev/null
		sleep 1
		pid=`cat $acu_pid 2>/dev/null`
		if [ "$pid" != "" ]; then
			echo "OK: $acu_name"
		else
			echo "FAILED: $acu_name; see logs"
		fi
	fi
}

# Stop command
stop(){
	pid=`cat $acu_pid 2>/dev/null`
	if [ "$pid" = "" ]; then
		echo "$acu_name is not running (pid file is empty)"
	else
		kill $pid
		rm -f $acu_pid 2>/dev/null
		echo "OK: $acu_name"
	fi
}

# Restart command
restart(){
	stop
	sleep 2
	start
}

run_rc_command "$1"
