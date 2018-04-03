GPP_FLAGS    = -std=c++11 -g
OBJECT_LIST  = main.o server.o Socket.o tcpSocket.o HTTPServer.o HTTPMessage.o
HEADER_FILES = Socket_Set.h
THREAD_LIB   = -lpthread


httpServer: $(OBJECT_LIST)
	g++ $(GPP_FLAGS) $(OBJECT_LIST) -o httpServer $(THREAD_LIB)
main.o: main.cpp
	g++ $(GPP_FLAGS) -c main.cpp
server.o: server.cpp server.h $(HEADER_FILES)
	g++ $(GPP_FLAGS) -c server.cpp
Socket.o: Socket.cpp Socket.h
	g++ $(GPP_FLAGS) -c Socket.cpp
tcpSocket.o: tcpSocket.cpp tcpSocket.h
	g++ $(GPP_FLAGS) -c tcpSocket.cpp
HTTPServer.o: HTTPServer.cpp HTTPServer.h
	g++ $(GPP_FLAGS) -c HTTPServer.cpp
HTTPMessage.o: HTTPMessage.cpp HTTPMessage.h
	g++ $(GPP_FLAGS) -c HTTPMessage.cpp
clean:
	rm $(OBJECT_LIST)
