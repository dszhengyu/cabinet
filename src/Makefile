CABINET_SERVER_NAME=cabinet-server
CABINET_SERVER_OBJECT=cabinet-server.o Server.o Log.o GetCommand.o CommandKeeper.o Client.o Util.o
CABINET_COMMON_HEADER=Command.h Const.h
CXXFLAGS= -std=c++11 -Wall -fPIC 
CXX=g++

$(CABINET_SERVER_NAME): $(CABINET_SERVER_OBJECT)
	$(CXX) -o $@ $^ $(CXXFLAGS)

%.o: %.cpp %.h $(CABINET_COMMON_HEADER)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.PHONY: clean cleanlog
clean:
	-rm -f $(CABINET_SERVER_NAME) $(CABINET_SERVER_OBJECT)
cleanlog:
	-rm -rf *.log
