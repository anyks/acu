#!/usr/bin/env bash

# $ chmod ugo+x /usr/local/share/acu/monitor/acu.sh
# $ sudo crontab -e
# */5 * * * * /usr/local/share/acu/monitor/acu.sh

ps -A | grep acu-server || service acu start
