CPPFLAGS=`libpng-config --cflags` -std=c++2a -O3 -Wall -Wextra -pedantic -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wlogical-op -Wshift-overflow=2 -Wduplicated-cond -Wcast-qual -Wcast-align -Wno-unused-result -Wno-sign-conversion -g
LDFLAGS=`libpng-config --ldflags` -O3 -g

main: main.o imagetexture.o
	g++ -o main imagetexture.o main.o $(LDFLAGS)

main.o: main.cpp imagetexture.hpp
	g++ -c main.cpp -o main.o $(CPPFLAGS)

imagetexture.o: imagetexture.cpp imagetexture.hpp
	g++ -c imagetexture.cpp -o imagetexture.o $(CPPFLAGS)

educational: main.o eduimagetexture.o
	g++ -o educational eduimagetexture.o main.o $(LDFLAGS)

eduimagetexture.o: eduimagetexture.cpp imagetexture.hpp
	g++ -c eduimagetexture.cpp -o eduimagetexture.o $(CPPFLAGS)

clean:
	rm -f main educational main.o imagetexture.o log