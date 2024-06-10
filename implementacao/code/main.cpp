#include "imagetexture.h"
#include <unistd.h>  //just for debug
#include <cstdlib> // just for debug


int main(){
    try{
        ImageTexture texture(300, 300);
        for(int i = 0; i < 13 * 13; i++){
            std::cout<<"ITERATION "<<i<<" ------------------------------- "<<std::endl;
            texture.patchFittingIteration("../input/eocystites_texture.png");
            //usleep(400000);
            texture.render("../output/eocystites_texture.png");
        }
    } catch(...){
        std::cout<<"exception"<<std::endl;
    }
}