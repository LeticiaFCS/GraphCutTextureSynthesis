#include "imagetexture.h"
#include <unistd.h>  //just for debug
#include <cstdlib> // just for debug


int main(){
    ImageTexture texture(300, 300);
    for(int i = 0; i < 13 * 13; i++){
        std::cout<<"ITERATION "<<i<<" ------------------------------- "<<std::endl;
        texture.patchFittingIteration("../input/areia_da_praia.png");
        //usleep(400000);
        texture.render("../output/areia_da_praia.png");
    }
}