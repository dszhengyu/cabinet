#!/bin/bash
#set -x

server_pid=`ps -ef | grep "cabinet-server" | grep -v "tail" | grep -v "grep" |awk '{print $2}' `
kill ${server_pid}
