#include "imagetexture.h"


int main(){
    ImageTexture texture(300, 300);
    for(int i = 0; i < 170; i++){
        texture.patchFittingIteration("../input/areia_da_praia.png");
        texture.render("../output/areia_da_praia.png");
    }
}