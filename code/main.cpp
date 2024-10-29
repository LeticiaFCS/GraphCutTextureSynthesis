#include "imagetexture.hpp"

int main(int argc, char *argv[]){
    // creates the image texture with dimensions 500x500
    ImageTexture texture(250, 250);

    // runs 500 iterations of the patch fitting using
    // "../input/areia_da_praia.png" as the input image
    texture.patchFitting("../input/areia_da_praia.png", 500);

    // renders the texture to the file "../output/output.png"
    texture.render("../output/output.png");
}

