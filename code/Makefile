CXXFLAGS=`libpng-config --cflags` -std=c++2a -O3 -Wall -Wextra -pedantic -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wlogical-op -Wshift-overflow=2 -Wduplicated-cond -Wcast-qual -Wcast-align -Wno-unused-result -Wno-sign-conversion -g
LDFLAGS=`libpng-config --ldflags` -O3 -g
mainfile = main.cpp
outputobj = main

main: main.o imagetexture.o
	g++ -o $(outputobj) imagetexture.o main.o $(LDFLAGS)

main.o: $(mainfile) imagetexture.hpp
	g++ -c $(mainfile) -o main.o $(CXXFLAGS)

imagetexture.o: imagetexture.cpp imagetexture.hpp
	g++ -c imagetexture.cpp -o imagetexture.o $(CXXFLAGS)

visual: main.o visualimagetexture.o
	g++ -o visual visualimagetexture.o main.o $(LDFLAGS)

visualimagetexture.o: visualimagetexture.cpp imagetexture.hpp
	g++ -c visualimagetexture.cpp -o visualimagetexture.o $(CXXFLAGS)

slow: main.o slowimagetexture.o
	g++ -o slow slowimagetexture.o main.o $(LDFLAGS)

slowimagetexture.o: imagetexture_slow_matching.cpp imagetexture.hpp
	g++ -c imagetexture_slow_matching.cpp -o slowimagetexture.o $(CXXFLAGS)

clean:
	rm -f $(outputobj) visual slow main.o imagetexture.o visualimagetexture.o slowimagetexture.o log