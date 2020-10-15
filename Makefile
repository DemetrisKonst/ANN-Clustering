LSH: interface_utils.o interface.o LSH_interface.o HCUBE_interface.o main.o
	g++ object/LSH_interface.o object/HCUBE_interface.o object/interface_utils.o object/interface.o object/main.o -o bin/LSH

interface_utils.o:
	g++ -c -o object/interface_utils.o src/interfaces/interface_utils.cpp

interface.o:
	g++ -c -o object/interface.o src/interfaces/interface.cpp

LSH_interface.o:
	g++ -c -o object/LSH_interface.o src/interfaces/LSH_interface.cpp

HCUBE_interface.o:
	g++ -c -o object/HCUBE_interface.o src/interfaces/HCUBE_interface.cpp

main.o:
	g++ -c -o object/main.o exc1/main.cpp
