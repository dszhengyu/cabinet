#Final Target
CABINET_FINAL_TARGET=$(CABINET_SERVER_NAME) $(CABINET_CLIENT_NAME)
CABINET_SERVER_NAME=cabinet-server
CABINET_CLIENT_NAME=cabinet-cli

#Common Dependency
CABINET_COMMAND_OBJECT=GetCommand.o NoMatchCommand.o
CABINET_COMMON_HEADER=Command.h Const.h

#Target Dependency
CABINET_SERVER_OBJECT=cabinet-server.o Server.o Log.o CommandKeeper.o Client.o Util.o EventPoll.o $(CABINET_COMMAND_OBJECT)
CABINET_CLIENT_OBJECT=cabinet-cli.o CabinetCli.o

#Compile Arguements
CXXFLAGS= -std=c++11 -Wall -fPIC 
CXX=g++

.PHONY: all
all: $(CABINET_FINAL_TARGET)

$(CABINET_SERVER_NAME): $(CABINET_SERVER_OBJECT)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(CABINET_CLIENT_NAME): $(CABINET_CLIENT_OBJECT)
	$(CXX) -o $@ $^ $(CXXFLAGS)

%.o: %.cpp %.h $(CABINET_COMMON_HEADER)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.PHONY: clean cleanlog
clean:
	-rm -f $(CABINET_SERVER_NAME) $(CABINET_SERVER_OBJECT) \
		$(CABINET_CLIENT_NAME) $(CABINET_CLIENT__OBJECT)
cleanlog:
	-rm -rf *.log
