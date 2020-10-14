CC=g++

LSH: include/LSH/LSHFun.hpp
	g++ -o bin/LSH exc1/main.cpp

clean:
	rm bin/*
