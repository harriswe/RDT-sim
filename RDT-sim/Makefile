CC = g++
rdt: rdt.o student_code.o
	$(CC) rdt.o student_code.o -o rdt

rdt.o: rdt.c rdt.h
	$(CC) -Wall -c rdt.c

student_code.o: student_code.cpp rdt.h
	g++ -Wall -c student_code.cpp

clean:
	rm *.o rdt
