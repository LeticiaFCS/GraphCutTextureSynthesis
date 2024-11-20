#include "imagetexture.hpp"

int main(int argc, char *argv[]){
    // creates the image texture with dimensions 500x500
    ImageTexture texture(250, 250);

    // runs 500 iterations of the patch fitting using
    // "../input/areia_da_praia.png" as the input image
    for(int i=0; i<500; i++){
        texture.patchFittingIteration("../input/areia_da_praia.png");
        texture.render("../output/output.png");
        usleep(800000);
    }
    // renders the texture to the file "../output/output.png"
    texture.render("../output/output.png");
}

