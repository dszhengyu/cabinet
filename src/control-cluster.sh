#!/bin/bash
#set -x

######################################## global variable ########################################
template_file="cabinet.conf.template"
conf_file="cabinet.conf"
if [ ! -e $template_file ]; then
    echo "Can Not Find Conf Template"
    exit 1
fi

######################################## function ########################################
function startClusterNode() {
    id=$1
    echo "Starting No.${id} Cluster..."
    cluster_ip_conf="CLUSTER_IP_${id}"
    cluster_port_conf="CLUSTER_PORT_${id}"
    cluster_server_ip_conf="CLUSTER_SERVER_IP_${id}"
    cluster_server_port_conf="CLUSTER_SERVER_PORT_${id}"
    cluster_ip=`grep $cluster_ip_conf $template_file | awk -F '=' '{print $2}'`
    cluster_port=`grep $cluster_port_conf $template_file | grep -v CLUSTER_CLUSTER_PORT_ | awk -F '=' '{print $2}'`
    cluster_server_ip=`grep $cluster_server_ip_conf $template_file | awk -F '=' '{print $2}'`
    cluster_server_port=`grep $cluster_server_port_conf $template_file | awk -F '=' '{print $2}'`
    echo "  IP: $cluster_ip"
    echo "  Port: $cluster_port"
    echo "  Server_IP: $cluster_server_ip"
    echo "  Server_Port: $cluster_server_port"

    sed "s/CLUSTER_ID_PLACEHOLDER/${id}/g" $template_file > $conf_file
    sed "s/CLUSTER_PORT_PLACEHOLDER/${cluster_port}/g" -i $conf_file
    sed "s/CLUSTER_SERVER_IP_PLACEHOLDER/${cluster_server_ip}/g" -i $conf_file
    sed "s/CLUSTER_SERVER_PORT_PLACEHOLDER/${cluster_server_port}/g" -i $conf_file
    sed "s/SERVER_IP_PLACEHOLDER/${cluster_server_ip}/g" -i $conf_file
    sed "s/SERVER_PORT_PLACEHOLDER/${cluster_server_port}/g" -i $conf_file
    sed "s/SERVER_ID_PLACEHOLDER/${id}/g" -i $conf_file

    ./cabinet-server ${cluster_server_port}
    ./cabinet-cluster id=${id} port=${cluster_port}
    
    echo ""
}

function stopClusterNode() {
    id=$1
    cluster_port_conf="CLUSTER_PORT_${id}"
    cluster_port=`grep $cluster_port_conf $template_file | grep -v CLUSTER_CLUSTER_PORT_ | awk -F '=' '{print $2}'`
    cluster_pid=`ps -ef | grep "cabinet" | grep "id=${id}" | grep "port=${cluster_port}" | \
                grep -v "tail" | grep -v "grep" |awk '{print $2}' `
    for pid in ${cluster_pid}
    do
        echo "Killing No.${id} Cluster..."
        echo "  pid=${pid}"
        echo "  port=${cluster_port}"
        kill -KILL $pid
    done

    cluster_server_port_conf="CLUSTER_SERVER_PORT_${id}"
    cluster_server_port=`grep $cluster_server_port_conf $template_file | awk -F '=' '{print $2}'`
    cluster_server_pid=`ps -ef | grep "cabinet" | grep "${cluster_server_port}" | grep -v "tail" | grep -v "grep" |awk '{print $2}' `
    for pid in ${cluster_server_pid}
    do
        echo "  server_pid=${pid}"
        echo "  server_port=${cluster_server_port}"
        echo ""
        kill -KILL $pid
    done
}

function startAllClusterNode() {
    cluster_id_min=`grep CLUSTER_ID_MIN $template_file | awk -F '=' '{print $2}'`
    cluster_id_max=`grep CLUSTER_ID_MAX $template_file | awk -F '=' '{print $2}'`
    for id in `seq ${cluster_id_min} ${cluster_id_max}`
    do
        startClusterNode ${id}
    done
}

function stopAllClusterNode() {
    cluster_id_min=`grep CLUSTER_ID_MIN $template_file | awk -F '=' '{print $2}'`
    cluster_id_max=`grep CLUSTER_ID_MAX $template_file | awk -F '=' '{print $2}'`
    for id in `seq ${cluster_id_min} ${cluster_id_max}`
    do
        stopClusterNode ${id}
    done
}

function usage() {
    echo "Usage: "
    echo "  ./constrol_cluster [start|stop] [0-9]{0,1}"
}
######################################## script ########################################

if [ $# -eq 0 -o $# -gt 2 ]; then
    usage
    exit 1
fi

if [ $1 == "start" ]; then
    if [ $# -eq 1 ]; then
        startAllClusterNode
        exit 0
    fi

    startClusterNode $2
    exit 0
fi
    

if [ $1 == "stop" ]; then
    if [ $# -eq 1 ]; then
        stopAllClusterNode
        exit 0
    fi

    stopClusterNode $2
    exit 0
fi

usage
exit 1
