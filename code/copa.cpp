#include "imagetexture.hpp"

int main(int argc, char *argv[]){
    // creates the image texture with dimensions 500x500
    ImageTexture texture(483, 309);

    // runs 500 iterations of the patch fitting using
    // "../input/areia_da_praia.png" as the input image
    std::vector<std::pair<int, int>> positions = {
        {0,0},
        {85,5},
        {170,10},

        {-8,173},
        {79,178},
        {164,183},

        {-16,346},
        {71,351},
        {156,356},

        {6,-173},
        {91,-168},
        {176,-163},
    };
    int i = 0;
    for(auto [a, b] : positions){
        std::cout<<"Iteração "<<i<<std::endl;
        i++;
        texture.blending(a, b, "../input_images/calcadao_input.png");
        texture.render("../output_images/output.png");
    }

    // renders the texture to the file "../output_images/output.png"
}

