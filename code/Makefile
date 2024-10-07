CPPFLAGS=`libpng-config --cflags` -std=c++2a -O3 -Wall -Wextra -pedantic -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wlogical-op -Wshift-overflow=2 -Wduplicated-cond -Wcast-qual -Wcast-align -Wno-unused-result -Wno-sign-conversion -g
LDFLAGS=`libpng-config --ldflags` -O3 -g

main: main.o imagetexture.o
	g++ -o main imagetexture.o main.o $(LDFLAGS)

main.o: main.cpp imagetexture.hpp
	g++ -c main.cpp -o main.o $(CPPFLAGS)

imagetexture.o: imagetexture.cpp imagetexture.hpp
	g++ -c imagetexture.cpp -o imagetexture.o $(CPPFLAGS)

visual: main.o visualimagetexture.o
	g++ -o visual visualimagetexture.o main.o $(LDFLAGS)

visualimagetexture.o: visualimagetexture.cpp imagetexture.hpp
	g++ -c visualimagetexture.cpp -o visualimagetexture.o $(CPPFLAGS)

clean:
	rm -f main visual main.o imagetexture.o visualimagetexture.o log