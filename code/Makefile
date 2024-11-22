## ----------------------------------------------------------------------
## Your code using the ImageTexture class should be in the file main.cpp
## inside the code folder.
##
## You can use either the fast version or the visual version of the
## class.
## To use the fast version compile using the command "make main" (or just 
## "make") and run with the command "./main"
## To use the visual version compile using the command "make visual" and
## run with the command "./visual"
##
## You can also compile only the object files of either implementation
## To compile the fast version use the command "make imagetexture.o"
## To compile the visual version use the command "make visualimagetexture.o"
##
## The visual version will render the output image in the file
## "output_images/output.png" after every iteration of the patch fitting.
## It will also render, in blue, the format of the new patch and pause for
## 0.8 seconds before copying the pixels from the new patch.
##
## ----------------------------------------------------------------------

CXXFLAGS=`libpng-config --cflags` -std=c++2a -O3 -Wall -Wextra -pedantic -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wlogical-op -Wshift-overflow=2 -Wduplicated-cond -Wcast-qual -Wcast-align -Wno-unused-result -Wno-sign-conversion -g
LDFLAGS=`libpng-config --ldflags` -O3 -g
mainfile = main.cpp
outputobj = main

main: main.o imagetexture.o	## Compile and link your code and the fast implementation of the class
	g++ -o $(outputobj) imagetexture.o main.o $(LDFLAGS)

main.o: $(mainfile) imagetexture.hpp ## Compile only the object file of your code
	g++ -c $(mainfile) -o main.o $(CXXFLAGS)

imagetexture.o: imagetexture.cpp imagetexture.hpp ## Compile only the object file of the fast implementation of the class
	g++ -c imagetexture.cpp -o imagetexture.o $(CXXFLAGS)

visual: main.o visualimagetexture.o	## Compile and link your code and the visual implementation of the class
	g++ -o visual visualimagetexture.o main.o $(LDFLAGS)

visualimagetexture.o: visualimagetexture.cpp imagetexture.hpp ## Compile only the object file of the visual implementation of the class
	g++ -c visualimagetexture.cpp -o visualimagetexture.o $(CXXFLAGS)

clean: ## Remove the object files
	rm -f $(outputobj) visual main.o imagetexture.o visualimagetexture.o

help:	## Show this help.
	@sed -ne '/@sed/!s/## //p' $(MAKEFILE_LIST)