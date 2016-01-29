CABINET_SERVER_NAME=cainet-server
CABINET_SERVER_OBJECT=cabinet-server.o Server.o Log.o
CXXFLAGS= -std=c++11 -Wall -fPIC 
CXX=g++

$(CABINET_SERVER_NAME): $(CABINET_SERVER_OBJECT)
	$(CXX) -o $@ $^ $(CXXFLAGS)

%.o: %.cpp %.h
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.PHONY: clean cleanlog
clean:
	-rm -f $(CABINET_SERVER_NAME) $(CABINET_SERVER_OBJECT)
cleanlog:
	-rm -rf *.log
