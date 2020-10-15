CC=g++

LSH: LSHMain LSH_interface.o interface_utils.o interface.o
	$(CC) -o bin/LSH object/main.o object/LSH_interface.o object/interface_utils.o object/interface.o

LSHMain:
	$(CC) -c -o object/main.o exc1/main.cpp
  
LSHInterface.o:
	$(CC) -c -o object/LSHInterface.o src/interfaces/LSH/LSH_interface.cpp
  
HCUBE_interface.o:
	$(CC) -c -o object/HCUBE_interface.o src/interfaces/HCUBE_interface.cpp

interface.o:
	$(CC) -c -o object/interface.o src/interfaces/interface.cpp

interface_utils.o:
	$(CC) -c -o object/interface_utils.o src/interfaces/interface_utils.cpp

clean:
	rm bin/* object/*.o
