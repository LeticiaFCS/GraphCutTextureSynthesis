#include "imagetexture.h"
#include <unistd.h>  //just for debug
#include <cstdlib> // just for debug


int main(int argc, char *argv[]){
    int lim = 1;
    if(argc > 1){
        lim = atoi(argv[1]);
    }
    for(int it =0 ; it < lim; it++){
        std::cout<<"NEW IMAGE "<<it<<std::endl;
        std::cerr<<"NEW IMAGE "<<it<<std::endl;
        try{
            ImageTexture texture(300, 300);
            for(int i = 0; i < 500; i++){
                std::cerr<<"ITERATION "<<i<<" ------------------------------- "<<std::endl;
                std::cout<<"ITERATION "<<i<<" ------------------------------- "<<std::endl;
                texture.patchFittingIteration("../input/areia_da_praia.png");
                //texture.render("../output/output.png");
                if(argc > 2)
                    usleep(400000);
            }
        } catch(...){
            std::cout<<"exception"<<std::endl;
        }
    }
}