# cabinet
##Concept
There are three main role    
1. server, basic nosql database, set value and get value    
2. cluster, component which use raft consensus to achieve high availability and data consensus    
3. client, use to connect cluster or server    
For more detail, please glance [My Paper](/raw/master/基于Raft协议的分布式Nosql数据库集群实现.pdf)

##How to use
###Compile
    cd src && make all
###How to use 
    #config the cluster configuration
    vi src/cabinet.conf.template 

    #start all cluster node and server node
    src/control-cluster.sh start  

    #use client to connect the cluster or server node
    src/cabinet-cli port-of-server-of-cluster 

    #stop cluster to test consensus
    src/control-cluster.sh stop cluster-id-to-stop 

#More 
This is my undergraduate final paper project    
Thx for the trust and support from my mentor professor WangPW   