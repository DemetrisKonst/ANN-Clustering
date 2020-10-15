CC=g++

LSH: LSHMain LSHInterface.o
	$(CC) -o bin/LSH object/main.o object/LSHInterface.o

LSHMain:
	$(CC) -c -o object/main.o exc1/main.cpp

LSHInterface.o:
	$(CC) -c -o object/LSHInterface.o src/interfaces/LSH/LSH_interface.cpp

clean:
	rm bin/* object/*.o
