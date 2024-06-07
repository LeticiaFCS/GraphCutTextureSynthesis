main.cpp:
	g++ -c main.cpp `libpng-config --cflags` -o main.o -std=c++2a -O2 -Wall -Wextra -pedantic -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wlogical-op -Wshift-overflow=2 -Wduplicated-cond -Wcast-qual -Wcast-align -Wno-unused-result -Wno-sign-conversion -g
	g++ -o main main.o `libpng-config --ldflags` -g