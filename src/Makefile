#Final Target
CABINET_FINAL_TARGET=$(CABINET_CLUSTER_NAME) $(CABINET_SERVER_NAME) $(CABINET_CLIENT_NAME)
CABINET_CLUSTER_NAME=cabinet-cluster
CABINET_SERVER_NAME=cabinet-server
CABINET_CLIENT_NAME=cabinet-cli

#Common Dependency
CABINET_COMMAND_OBJECT=GetCommand.o SetCommand.o DelCommand.o NoMatchCommand.o CommandKeeper.o
CABINET_CLUSTER_COMMAND_OBJECT=CommandKeeper.o RequestVoteCommand.o ReplyRequestVoteCommand.o \
							   AppendEntryCommand.o ReplyAppendEntryCommand.o ClusterDefaultCommand.o \
							   FlushServerCommand.o ClusterNodeCommand.o ReplyClusterNodeCommand.o
CABINET_COMMON_HEADER=Command.h Const.h ValueObj.h CabinetLogo.h Cabinet.h Client.h
CABINET_COMMON_OBJECT=Log.o Util.o ProtocolStream.o Configuration.o
CABINET_DATABASE_OBJECT=DataBase.o StringObj.o

#Target Dependency
CABINET_CLUSTER_OBJECT=cabinet-cluster.o Cabinet.o Cluster.o Client.o ClusterClient.o EventPoll.o PersistenceFile.o Entry.o \
					   Siblings.o Children.o Parents.o \
						$(CABINET_COMMON_OBJECT) $(CABINET_CLUSTER_COMMAND_OBJECT)
CABINET_SERVER_OBJECT=cabinet-server.o Cabinet.o Server.o Client.o ServerClient.o EventPoll.o PersistenceFile.o Entry.o \
						$(CABINET_COMMON_OBJECT) $(CABINET_COMMAND_OBJECT) $(CABINET_DATABASE_OBJECT)
CABINET_CLIENT_OBJECT=cabinet-cli.o CabinetCli.o $(CABINET_COMMON_OBJECT) $(CABINET_COMMAND_OBJECT)

#Compile Arguements
CABINET_CLUSTER_CXXFLAGS= -std=c++11 -Wall -fPIC -DCABINET_CLUSTER -DCABINET
CABINET_SERVER_CXXFLAGS= -std=c++11 -Wall -fPIC -DCABINET_SERVER -DCABINET
CABINET_CLIENT_CXXFLAGS= -std=c++11 -Wall -fPIC -DCLIENT
CXX=g++

.PHONY: all
all:
	$(MAKE) $(CABINET_CLUSTER_NAME) CXXFLAGS="$(CABINET_CLUSTER_CXXFLAGS)"
	$(MAKE) $(CABINET_SERVER_NAME) CXXFLAGS="$(CABINET_SERVER_CXXFLAGS)"
	$(MAKE) $(CABINET_CLIENT_NAME) CXXFLAGS="$(CABINET_CLIENT_CXXFLAGS)"

cluster:
	$(MAKE) $(CABINET_CLUSTER_NAME) CXXFLAGS="$(CABINET_CLUSTER_CXXFLAGS)"

server:
	$(MAKE) $(CABINET_SERVER_NAME) CXXFLAGS="$(CABINET_SERVER_CXXFLAGS)"

cli:
	$(MAKE) $(CABINET_CLIENT_NAME) CXXFLAGS="$(CABINET_CLIENT_CXXFLAGS)"
	
$(CABINET_CLUSTER_NAME): $(CABINET_CLUSTER_OBJECT)
	$(CXX) -o $@ $^ $(CXXFLAGS)
	@-rm -f $(CABINET_COMMAND_OBJECT)

$(CABINET_SERVER_NAME): $(CABINET_SERVER_OBJECT)
	$(CXX) -o $@ $^ $(CXXFLAGS)
	@-rm -f $(CABINET_COMMAND_OBJECT)

$(CABINET_CLIENT_NAME): $(CABINET_CLIENT_OBJECT)
	$(CXX) -o $@ $^ $(CXXFLAGS)
	@-rm -f $(CABINET_COMMAND_OBJECT)

%.o: %.cpp %.h $(CABINET_COMMON_HEADER)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.PHONY: clean cleanlog
clean:
	-rm -f $(CABINET_FINAL_TARGET) *.o
cleanlog:
	-rm -rf *.log
