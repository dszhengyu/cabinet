#!/bin/bash
#set -x

cluster_pid=`ps -ef | grep "cabinet" | grep -v "tail" | grep -v "grep" |awk '{print $2}' `
for pid in ${cluster_pid}
do
    kill $pid
done
