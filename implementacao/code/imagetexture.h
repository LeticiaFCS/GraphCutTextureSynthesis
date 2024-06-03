#include <png++/png.hpp>
#include <algorithm>
#include <math.h>
#include <utility>
#include <array>
#include <queue>
#include <tuple>
#include <unistd.h>  //just for debug
#include <cstdlib> // just for debug
#include <map>

class ImageTexture{
private:
    png::image<png::rgb_pixel> img;
    const int imgWidht;
    const int imgHeight;
    enum PixelStatus{
        colored, intersection, newcolor, notcolored
    };
    std::vector<std::vector<PixelStatus>> pixelColorStatus;
    template<typename Pixel>
    static long double calcCost(const Pixel &as, const Pixel &bs, const Pixel &at, const Pixel &bt);
    template<typename T>
    void clean_graph(std::vector<std::vector<T>> &g);
    std::pair<int, int> matching();
    bool stPlanarGraph(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    void blending(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
public:
    ImageTexture(const png::image<png::rgb_pixel> & _img);
    ImageTexture(int width, int height);
    void render(const std::string &file_name);
    void patchFittingIteration(const png::image<png::rgb_pixel> &inputImg);
    void patchFittingIteration(const std::string &file_name);
    void patchFitting(const png::image<png::rgb_pixel> &inputImg);
    void patchFitting(const std::string &file_name);
};

/*
Constructors
*/
ImageTexture::ImageTexture(const png::image<png::rgb_pixel> & _img) 
    : img(_img), 
    imgWidht(_img.get_width()),
    imgHeight(_img.get_height()),
    pixelColorStatus(_img.get_width(), std::vector<ImageTexture::PixelStatus>(_img.get_height(), ImageTexture::PixelStatus::notcolored)){   
};
ImageTexture::ImageTexture(int width, int height) 
    : img(width, height), 
    imgWidht(width),
    imgHeight(height), 
    pixelColorStatus(width, std::vector<ImageTexture::PixelStatus>(height, ImageTexture::PixelStatus::notcolored)){    
};

/*
Public Functions
*/
void ImageTexture::render(const std::string &file_name){
    img.write(file_name);
};

void ImageTexture::patchFitting(const png::image<png::rgb_pixel> &inputImg){

}
void ImageTexture::patchFitting(const std::string &file_name){
    ImageTexture::patchFitting(png::image<png::rgb_pixel>(file_name));
}
void ImageTexture::patchFittingIteration(const png::image<png::rgb_pixel> &inputImg){
    const auto [heightOffset, widthOffset] = ImageTexture::matching();
    this->ImageTexture::blending(heightOffset, widthOffset, inputImg);
    usleep(40000);
    std::cout<<widthOffset<<" "<<heightOffset<<"\n";
}
void ImageTexture::patchFittingIteration(const std::string &file_name){
    ImageTexture::patchFittingIteration(png::image<png::rgb_pixel>(file_name));
}

/*
Private Functions
*/
template<typename Pixel>
long double ImageTexture::calcCost(const Pixel &as, const Pixel &bs, const Pixel &at, const Pixel &bt){
    long double cost =
    sqrtl(pow2(as.red-bs.red) + pow2(as.green-bs.green) + pow2(as.blue-bs.blue)) +
    sqrtl(pow2(at.red-bt.red) + pow2(at.green-bt.green) + pow2(at.blue-bt.blue));
    return cost;
};
template<typename T>
void ImageTexture::clean_graph(std::vector<std::vector<T>> &g){
    using namespace std;
    for(int i = 0; i < (int)g.size(); i++){
        sort(g[i].begin(), g[i].end());
        g[i].erase(unique(g[i].begin(), g[i].end()), g[i].end());
    }
};

std::pair<int, int> ImageTexture::matching(){
    static int heightOffset = 0;
    static int widthOffset = -24;
    widthOffset += 24;
    if(widthOffset >= this->imgWidht){
        widthOffset = 0;
        heightOffset += 24;
    }
    if(heightOffset >= this->imgHeight){
        heightOffset = 0;
        widthOffset = 0;
    }
    return {heightOffset, widthOffset};
}

bool ImageTexture::stPlanarGraph(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    return true;
}
void ImageTexture::blending(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    for(int i = 0; i < inputImg.get_height() && i + heightOffset < this->imgHeight; i++){
        for(int j = 0; j < inputImg.get_width() && j + widthOffset < this->imgWidht; j++){

        }
    }
}
// Naive Blending
// void ImageTexture::blending(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
//     for(int i = 0; i < inputImg.get_height() && i + heightOffset < this->imgHeight; i++){
//         for(int j = 0; j < inputImg.get_width() && j + widthOffset < this->imgWidht; j++){
//             this->img[i + heightOffset][j + widthOffset] = inputImg[i][j];
//             this->pixelColorStatus[i][j] = ImageTexture::PixelStatus::colored;
//         }
//     }
// }