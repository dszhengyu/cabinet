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
#find and modify cluster conf
    cluster_ip_conf="CLUSTER_IP_${id}"
    cluster_port_conf="CLUSTER_PORT_${id}"
    cluster_ip=`grep $cluster_ip_conf $template_file | awk -F '=' '{print $2}'`
    cluster_port=`grep $cluster_port_conf $template_file | grep -v CLUSTER_CLUSTER_PORT_ | awk -F '=' '{print $2}'`
    sed "s/CLUSTER_ID_PLACEHOLDER/${id}/g" $template_file > $conf_file
    sed "s/CLUSTER_PORT_PLACEHOLDER/${cluster_port}/g" -i $conf_file
    echo "  IP: $cluster_ip"
    echo "  Port: $cluster_port"

#find server conf
    cluster_server_ip_conf="CLUSTER_SERVER_IP-${id}"
    cluster_server_port_conf="CLUSTER_SERVER_PORT-${id}"
    cluster_server_ip=`grep $cluster_server_ip_conf $template_file | awk -F '=' '{print $2}'`
    cluster_server_port=($(grep $cluster_server_port_conf $template_file | awk -F '=' '{printf("%d ", $2)}'))
    server_total=0
    server_id=1
    for server_ip in ${cluster_server_ip}
    do
        server_port=${cluster_server_port[$server_total]}
#modify server conf for cluster
        sed "s/CLUSTER_SERVER_IP_${server_id}_PLACEHOLDER/${server_ip}/g" -i $conf_file
        sed "s/CLUSTER_SERVER_PORT_${server_id}_PLACEHOLDER/${server_port}/g" -i $conf_file

#modify server conf for server self
        lineNo=`grep ^SERVER_PORT $conf_file -n | awk -F : '{print $1}'`
        sed "${lineNo}c SERVER_PORT=${server_port}" -i $conf_file

#echo message, launch server
        echo "  Server_IP_${server_id}: $server_ip"
        echo "  Server_Port_${server_id}: $server_port"
        ./cabinet-server cluster_id=${id} port=${server_port}

        server_total=$(($server_total + 1))
        server_id=$((server_id + 1))
    done

#launch cluster
    sed "s/CLUSTER_SERVER_TOTAL_PLACEHOLDER/${server_total}/g" -i $conf_file
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

    cluster_server_port_conf="CLUSTER_SERVER_PORT-${id}"
    cluster_server_port=`grep $cluster_server_port_conf $template_file | awk -F '=' '{print $2}'`
    for server_port in ${cluster_server_port}
    do
        cluster_server_identifier="cluster_id=${id} port=${server_port}"
        cluster_server_pid=`ps -ef | grep "${cluster_server_identifier}" | grep -v "tail" | grep -v "grep" |awk '{print $2}' `
        for pid in ${cluster_server_pid}
        do
            echo "  server_pid=${pid}"
            echo "  server_port=${server_port}"
            kill -KILL $pid
        done
    done
    echo ""
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
