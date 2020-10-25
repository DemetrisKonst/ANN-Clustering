CC=g++

# used to compile all programs
# ALL: CLUSTER HC LSH

# compile clustering
CLUSTER: ClusterMain clusteringInterface.o interface_utils.o interface.o
	$(CC) -o bin/cluster object/ClusterMain.o object/clustering_interface.o object/interface_utils.o object/interface.o

ClusterMain:
	$(CC) -c -o object/ClusterMain.o exc2/ClusterMain.cpp

clusteringInterface.o:
	$(CC) -c -o object/clustering_interface.o src/interfaces/clustering_interface.cpp

# compile Hypercube
HC: HCMain HCInterface.o interface_utils.o interface.o
	$(CC) -o bin/HC object/HCMain.o object/HCInterface.o object/interface_utils.o object/interface.o

HCMain:
	$(CC) -c -o object/HCMain.o exc1/HCMain.cpp

HCInterface.o:
	$(CC) -c -o object/HCInterface.o src/interfaces/HC_interface.cpp

# compile LSH
LSH: LSHMain LSHInterface.o interface_utils.o interface.o
	$(CC) -o bin/LSH object/LSHMain.o object/LSH_interface.o object/interface_utils.o object/interface.o

LSHMain:
	$(CC) -c -o object/LSHMain.o exc1/LSHMain.cpp

LSHInterface.o:
	$(CC) -c -o object/LSH_interface.o src/interfaces/LSH_interface.cpp

interface.o:
	$(CC) -c -o object/interface.o src/interfaces/interface.cpp

interface_utils.o:
	$(CC) -c -o object/interface_utils.o src/interfaces/interface_utils.cpp

clean:
	rm bin/* object/*.o
