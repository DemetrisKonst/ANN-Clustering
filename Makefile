CC=g++
CFLAGS=-O3

# used to compile all programs
ALL: CLUSTER HC LSH

# compile clustering
CLUSTER: ClusterMain clusteringInterface.o interface_utils.o interface.o
	$(CC) -o bin/cluster object/ClusterMain.o object/clustering_interface.o object/interface_utils.o object/interface.o $(CFLAGS)

ClusterMain:
	$(CC) -c -o object/ClusterMain.o Clustering/ClusterMain.cpp $(CFLAGS)

clusteringInterface.o:
	$(CC) -c -o object/clustering_interface.o src/interfaces/clustering_interface.cpp $(CFLAGS)

# compile Hypercube
HC: HCMain HCInterface.o interface_utils.o interface.o
	$(CC) -o bin/cube object/HCMain.o object/HCInterface.o object/interface_utils.o object/interface.o $(CFLAGS)

HCMain:
	$(CC) -c -o object/HCMain.o ANN/HCMain.cpp $(CFLAGS)

HCInterface.o:
	$(CC) -c -o object/HCInterface.o src/interfaces/HC_interface.cpp $(CFLAGS)

# compile LSH
LSH: LSHMain LSHInterface.o interface_utils.o interface.o
	$(CC) -o bin/lsh object/LSHMain.o object/LSH_interface.o object/interface_utils.o object/interface.o $(CFLAGS)

LSHMain:
	$(CC) -c -o object/LSHMain.o ANN/LSHMain.cpp $(CFLAGS)

LSHInterface.o:
	$(CC) -c -o object/LSH_interface.o src/interfaces/LSH_interface.cpp $(CFLAGS)

interface.o:
	$(CC) -c -o object/interface.o src/interfaces/interface.cpp $(CFLAGS)

interface_utils.o:
	$(CC) -c -o object/interface_utils.o src/interfaces/interface_utils.cpp $(CFLAGS)

clean:
	rm bin/* object/*.o
