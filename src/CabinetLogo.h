#ifndef CABINET_CLUSTER
const char *cabinet_server_logo = 
"\n__________\n"
"|        |\n"   
"|        |\n"   
"|        |\n"   
"|     || |    Cabinet-server\n"   
"|        |    Port: %d\n"   
"|        |    Persistence: %s\n"   
"+________+\n";
#endif

#ifdef CABINET_CLUSTER
const char *cabinet_cluster_logo = 
"\n____________________________\n"
"|        |        |        |\n"   
"|        |        |        |\n"   
"|        |        |        |\n"   
"|     || |     || |     || |\n"   
"|        |        |        |\n"   
"|        |        |        |\n"   
"+________+________+________+\n"
"|        |        |        |\n"   
"|        |        |        |\n"   
"|        |        |        |\n"   
"|     || |     || |     || |    Cabinet-cluster\n"   
"|        |        |        |    Port: %d\n"   
"|        |        |        |    PersistenceFileName: %s\n"
"+________+________+________+    Cluster_id: %d\n"
"|        |        |        |    Hz: %d\n"   
"|        |        |        |    ElectionTimeout: %d\n"   
"|        |        |        |\n"   
"|     || |     || |     || |\n"   
"|        |        |        |\n"   
"|        |        |        |\n"   
"+________+________+________+\n";
#endif
