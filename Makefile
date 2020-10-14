LSH: LSH_interface.o main.o
	g++ object/LSH_interface.o object/main.o -o bin/LSH

LSH_interface.o:
	g++ -c -o object/LSH_interface.o src/interfaces/LSH/LSH_interface.cpp

main.o:
	g++ -c -o object/main.o exc1/main.cpp
