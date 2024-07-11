#include "imagetexture.h"
#include <unistd.h>  //just for debug
#include <cstdlib> // just for debug


int main(int argc, char *argv[]){
    int lim;
    std::cin>>lim;
    for(int it =0 ; it < lim; it++){
        std::cout<<"NEW IMAGE "<<it<<std::endl;
        std::cerr<<"NEW IMAGE "<<it<<std::endl;
        try{
            ImageTexture texture(300, 300);
            for(int i = 0; i < 1200/*13*13*/; i++){
                std::cout<<"ITERATION "<<i<<" ------------------------------- "<<std::endl;
                texture.patchFittingIteration("../input/fur.png");
                texture.render("../output/output.png");
                if(argc > 1)
                    usleep(400000);
            }
        } catch(...){
            std::cout<<"exception"<<std::endl;
        }
    }
}