#include "imagetexture.hpp"

int main(int argc, char *argv[]){
    // creates the image texture with dimensions 500x500
    ImageTexture texture(320, 240);

    // runs 500 iterations of the patch fitting using
    // "../input/areia_da_praia.png" as the input image
    std::vector<std::pair<int, int>> positions;
    for(int i = 0; i <13; i++){
        for(int j = 0; j < 15; j++){
            positions.emplace_back(i*17, j*20);
        }
    }
    for(auto [a, b] : positions){
        texture.blending(a, b, "../input_images/muro0.png");
    } 
    texture.render("../output_images/muro_manual.png");

    png::image<png::rgb_pixel> inImg("../input_images/muro0.png");
    png::image<png::rgb_pixel> outImg(320,240);
    for(auto [a, b] : positions){
        for(int i = 0; i < inImg.get_height(); i++)
            for(int j = 0; j < inImg.get_width(); j++){
                if(0 <= a+i && a+i < outImg.get_height()
                && 0 <= b+j && b+j < outImg.get_width())
                    outImg[a+i][b+j] = inImg[i][j];
            }
    }
    outImg.write("../output_images/muro_sem_blending.png");
}

