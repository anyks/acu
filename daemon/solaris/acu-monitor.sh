#!/usr/bin/env bash

# $ chmod ugo+x /usr/local/share/acu/monitor/acu.sh
# $ sudo crontab -e
# */5 * * * * /usr/local/share/acu/monitor/acu.sh

# Check server
readonly SERVER=$(/usr/bin/ps -A | /usr/bin/ggrep acu-server | /usr/bin/gsed -n '2p')

# If Server is not run
if [ ! -n "$SERVER" ]; then
	# Run this server
	/etc/init.d/acu start
fi
