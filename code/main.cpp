#include "imagetexture.hpp"

int main(int argc, char *argv[]){
    // creates the image texture with dimensions 500x500
    ImageTexture texture(600, 600);

    // runs 500 iterations of the patch fitting using
    // "../input/areia_da_praia.png" as the input image
    for(int i = 0; i < 500; i++){
        if(i % 4 == 0){
            texture.patchFittingIteration("../input/cafezinho_grande.png");
        } else if(i % 4 == 1){
            texture.patchFittingIteration("../input/cafezinho_grande90.png");
        } else if(i % 4 == 2){
            texture.patchFittingIteration("../input/cafezinho_grande180.png");
        } else if(i % 4 == 3){
            texture.patchFittingIteration("../input/cafezinho_grande270.png");
        } 
    }

    // renders the texture to the file "../output_images/output.png"
    texture.render("../output_images/output.png");
}

