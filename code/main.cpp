#include "imagetexture.hpp"

int main(){
    // creates the image texture with dimensions 500x500
    ImageTexture texture(640, 427);

    // runs 500 iterations of the patch fitting using
    // "../input/areia_da_praia.png" as the input image
    for(int i = 0; i < 300; i++){
        if(i%2 == 0)
            texture.patchFittingIteration("../input_images/jeans_input0.png");
        else
            texture.patchFittingIteration("../input_images/jeans_input1.png");
    }

    // renders the texture to the file "../output_images/output.png"
    texture.render("../output_images/output.png");
}

