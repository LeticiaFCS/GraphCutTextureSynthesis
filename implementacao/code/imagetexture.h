#include <png++/png.hpp>
#include <algorithm>
#include <math.h>
#include <utility>
#include <array>
#include <queue>
#include <tuple>
#include <map>
#include <cassert>

#include<string>
std::string printEnum(int x){ // just to debug
    if(x == 0) return "colored";
    if(x == 1) return "intersection";
    if(x == 2) return "newcolor";
    if(x == 3) return "not colored";
    assert(("X should be between 0 and 3", false));
}

class ImageTexture{
public:
    ImageTexture(const png::image<png::rgb_pixel> & _img);
    ImageTexture(int width, int height);
    void render(const std::string &file_name);
    void patchFittingIteration(const png::image<png::rgb_pixel> &inputImg);
    void patchFittingIteration(const std::string &file_name);
    void patchFitting(const png::image<png::rgb_pixel> &inputImg);
    void patchFitting(const std::string &file_name);
private:
    enum PixelStatusEnum{
        colored, intersection, newcolor, notcolored
    };
    png::image<png::rgb_pixel> img;
    const int imgWidht;
    const int imgHeight;
    std::vector<std::vector<PixelStatusEnum>> pixelColorStatus;
    static constexpr std::array<std::pair<int, int>, 4> directions = {{
        {-1, 0},    //    |3|
        { 0,-1},    //  |0|x|2|
        { 1, 0},    //    |1|
        { 0, 1}     //
    }};
    static constexpr std::array<std::pair<int, int>, 4> primalToDual = {{
        { 0, 0},    //  0_3
        { 1, 0},    //  | |
        { 1, 1},    //  1_2
        { 0, 1}     //
    }};
    static constexpr std::array<std::pair<int, int>, 4> dualToPrimal = {{
        {-1,-1},    //  0|3
        { 0,-1},    //  -*-
        { 0, 0},    //  1|2 
        {-1, 0}     //
    }};
    static int nextDir(int i);
    static int prevDir(int i);
    template<typename Pixel>
    static long double calcCost(const Pixel &as, const Pixel &bs, const Pixel &at, const Pixel &bt);
    
    std::pair<int, int> matching();
    bool isFirstPatch(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    void copyFirstPatch(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    bool stPlanarGraph(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    void blendingCase1(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    void blendingCase2(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    void blending(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);

    void copyPixelsNewColor(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);    
    bool insidePrimal(int i, int j);    
    bool insideDual(int i, int j);
    class Intersection{
        public:
            // counter clockwise, starting with upper neighbor
            std::vector<std::pair<int, int>> interPixels;
            Intersection(const std::vector<std::pair<int,int>> &pixels = {}) : interPixels(pixels){}
    };
    std::pair<std::pair<int, int>, std::pair<int, int> > findSTInIntersection(Intersection &inter);
    std::vector<Intersection> findIntersections(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    void markMinABCut(std::pair<int, int> S, std::pair<int, int> T, const ImageTexture::Intersection &inter);

};

/*
Constructors
*/
ImageTexture::ImageTexture(const png::image<png::rgb_pixel> & _img) 
    : img(_img), 
    imgWidht(_img.get_width()),
    imgHeight(_img.get_height()),
    pixelColorStatus(_img.get_width(), std::vector<PixelStatusEnum>(_img.get_height(), PixelStatusEnum::notcolored)){
}
ImageTexture::ImageTexture(int width, int height) 
    : img(width, height), 
    imgWidht(width),
    imgHeight(height), 
    pixelColorStatus(width, std::vector<PixelStatusEnum>(height, PixelStatusEnum::notcolored)){    
}

/*
Public Functions
*/
void ImageTexture::render(const std::string &file_name){
    img.write(file_name);
}
void ImageTexture::patchFitting(const png::image<png::rgb_pixel> &inputImg){
    for(int i = 0 ; i < 100; i++)
        patchFittingIteration(inputImg);
}
void ImageTexture::patchFitting(const std::string &file_name){
    ImageTexture::patchFitting(png::image<png::rgb_pixel>(file_name));
}
void ImageTexture::patchFittingIteration(const png::image<png::rgb_pixel> &inputImg){
    const auto [heightOffset, widthOffset] = matching();
    std::cout<<heightOffset<<" "<<widthOffset<<"\n";
    if(isFirstPatch(heightOffset, widthOffset, inputImg))
        copyFirstPatch(heightOffset, widthOffset, inputImg);
    else
        blending(heightOffset, widthOffset, inputImg);
}
void ImageTexture::patchFittingIteration(const std::string &file_name){
    patchFittingIteration(png::image<png::rgb_pixel>(file_name));
}
/*
Private Functions
*/

// Main Private Functions
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
    std::cout<<"MATCHING "<<heightOffset<<" "<<widthOffset<<std::endl;
    return {heightOffset, widthOffset};
}
bool ImageTexture::isFirstPatch(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidht; j++, b++){
            if(this->pixelColorStatus[a][b] != PixelStatusEnum::notcolored){
                return false;
            }
        }
    return true;    
}
void ImageTexture::copyFirstPatch(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    std::cout<<"Copy First Patch"<<std::endl;
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidht; j++, b++){
            this->pixelColorStatus[a][b] = PixelStatusEnum::newcolor;
        }  
    copyPixelsNewColor(heightOffset, widthOffset, inputImg);
}
bool ImageTexture::stPlanarGraph(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    {//upper edge
        int upperEdgeHeight = heightOffset;
        for(int j = 0; j < (int) inputImg.get_width() && j + widthOffset < this->imgWidht; j++){
            if(this->pixelColorStatus[upperEdgeHeight][j + widthOffset] == this->PixelStatusEnum::notcolored)
                return true;
        }
        std::cout<<"upper edge is covered "<<upperEdgeHeight<<std::endl;
    }
    {//lower edge
        int lowerEdgeHeight = std::min<int>(heightOffset + inputImg.get_height() - 1 , this->imgHeight - 1);
        for(int j = 0; j < (int) inputImg.get_width() && j + widthOffset < this->imgWidht; j++){
            if(this->pixelColorStatus[lowerEdgeHeight][j + widthOffset] == this->PixelStatusEnum::notcolored)
                return true;
        }
        std::cout<<"lower edge is covered "<<lowerEdgeHeight<<std::endl;
    }    
    {//left edge
        int leftEdgeWidth = widthOffset;
        for(int i = 0; i < (int) inputImg.get_height() && i + heightOffset < this->imgHeight; i++){
            if(this->pixelColorStatus[i + heightOffset][leftEdgeWidth] == this->PixelStatusEnum::notcolored)
                return true;
        }
        std::cout<<"left edge is covered "<<leftEdgeWidth<<std::endl;
    }
    {//right edge
        int rightEdgeWidth = std::min<int>(widthOffset + inputImg.get_width() - 1, this->imgWidht - 1);
        for(int i = 0; i < (int) inputImg.get_height() && i + heightOffset < this->imgHeight; i++){
            if(this->pixelColorStatus[i + heightOffset][rightEdgeWidth] == this->PixelStatusEnum::notcolored)
                return true;
        }
        std::cout<<"right edge is covered "<<rightEdgeWidth<<std::endl;
    }
    return false;
}
void ImageTexture::blendingCase1(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    std::cout<<"Case 1"<<std::endl;
    std::vector<Intersection> intersections = findIntersections(heightOffset, widthOffset, inputImg);
    std::cout<<"numer of intersections "<<intersections.size()<<std::endl;
    for(auto &inter : intersections){
        std::cout<<"intersection size "<<inter.interPixels.size()<<std::endl;
        auto [S, T] = findSTInIntersection(inter);
        markMinABCut(S, T, inter);
    }
    copyPixelsNewColor(heightOffset, widthOffset, inputImg);
    
}
void ImageTexture::blendingCase2(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    std::cout<<"Case 2"<<std::endl;
    //Intersection intersection = findIntersections(heightOffset, widthOffset, inputImg)[0];
}
void ImageTexture::blending(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    std::cout<<"BLENDING"<<std::endl;
    if(this->stPlanarGraph(heightOffset, widthOffset, inputImg))
        this->blendingCase1(heightOffset, widthOffset, inputImg);
    else
        this->blendingCase2(heightOffset, widthOffset, inputImg);
}

// Auxiliar Static Functions
template<typename Pixel>
long double ImageTexture::calcCost(const Pixel &as, const Pixel &bs, const Pixel &at, const Pixel &bt){
    long double cost =
    sqrtl(pow2(as.red-bs.red) + pow2(as.green-bs.green) + pow2(as.blue-bs.blue)) +
    sqrtl(pow2(at.red-bt.red) + pow2(at.green-bt.green) + pow2(at.blue-bt.blue));
    return cost;
}

int ImageTexture::nextDir(int i){
    return (i + 1)% int(directions.size());
}
int ImageTexture::prevDir(int i){
    int j = (i - 1)% int(directions.size());
    if(j < 0)
        j += int(directions.size());
    return j;
}
// Auxiliar Nonstatic Functions
void ImageTexture::copyPixelsNewColor(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidht; j++, b++)
            if(pixelColorStatus[a][b] == PixelStatusEnum::newcolor){
                img[a][b] = inputImg[i][j];
                pixelColorStatus[a][b] = PixelStatusEnum::colored;
            }
}
bool ImageTexture::insidePrimal(int i, int j){
    return 0 <= i && i < imgHeight && 0 <= j && j < imgWidht;
}
bool ImageTexture::insideDual(int i, int j){
    return 0 <= i && i < imgHeight + 1 && 0 <= j && j < imgWidht + 1;
}
std::pair<std::pair<int, int>, std::pair<int, int> > ImageTexture::findSTInIntersection(ImageTexture::Intersection &inter){
    std::pair<int, int> S = {-1,-1}, T = {-1,-1};
    for(int pos = 0; pos < (int) inter.interPixels.size(); pos++){
        const auto &[i, j] = inter.interPixels[pos];
        for(int d = 0; d < (int) directions.size(); d++){
            int neiIA = i + directions[d].first;
            int neiJA = j + directions[d].second;   
            if(!insidePrimal(neiIA, neiJA)) continue;
            if(pixelColorStatus[neiIA][neiJA] != PixelStatusEnum::colored) continue;
            int prevD = prevDir(d);
            int neiIB = i + directions[prevD].first;
            int neiJB = j + directions[prevD].second;
            bool outsideB = !insidePrimal(neiIB, neiJB);
            if(outsideB || pixelColorStatus[neiIB][neiJB] == PixelStatusEnum::notcolored || pixelColorStatus[neiIB][neiJB] == PixelStatusEnum::newcolor){
                std::cout<<"Special "<<i<<" "<<j<<std::endl;
                S = {i + primalToDual[d].first, j + primalToDual[d].second};
            }
        }
        for(int d = 0; d < (int) directions.size(); d++){
            int neiIA = i + directions[d].first;
            int neiJA = j + directions[d].second;   
            if(!insidePrimal(neiIA, neiJA)) continue;
            if(pixelColorStatus[neiIA][neiJA] != PixelStatusEnum::colored) continue;
            int nextD = nextDir(d);
            int neiIB = i + directions[nextD].first;
            int neiJB = j + directions[nextD].second;   
            bool outsideB = !insidePrimal(neiIB, neiJB);        
            if(outsideB || pixelColorStatus[neiIB][neiJB] == PixelStatusEnum::notcolored || pixelColorStatus[neiIB][neiJB] == PixelStatusEnum::newcolor){
                std::cout<<"Special "<<i<<" "<<j<<std::endl;
                T = {i + primalToDual[nextD].first, j + primalToDual[nextD].second};
            }
        }
    }
    assert(("findSTInIntersection should always find S", (S) != (std::pair<int, int>{-1,-1})));
    assert(("findSTInIntersection should always find T", (T) != (std::pair<int, int>{-1,-1})));
    return {S, T};
}
std::vector<ImageTexture::Intersection> ImageTexture::findIntersections(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    static std::vector<std::vector<int>> lastVisit(imgHeight, std::vector<int>(imgWidht, 0));
    static int Time = 0;
    std::vector<Intersection> intersectionsList;
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidht; j++, b++)
            if(pixelColorStatus[a][b] == PixelStatusEnum::colored){
                Time++;
                std::vector<std::pair<int,int>> interPixels;
                pixelColorStatus[a][b] = PixelStatusEnum::intersection;
                interPixels.emplace_back(a,b);
                lastVisit[a][b] = Time;
                for(int front = 0; front < (int) interPixels.size(); front++){
                    auto [iFront, jFront] = interPixels[front];
                    for(auto [deltaI, deltaJ] : directions){
                        int nborI = iFront + deltaI;
                        int nborJ = jFront + deltaJ;
                        if(!insidePrimal(nborI, nborJ)) continue;
                        if(nborI - heightOffset >= (int) inputImg.get_height() || nborJ - widthOffset >= (int) inputImg.get_width()) continue;
                        if(nborI - heightOffset < 0 || nborJ - widthOffset < 0) continue;
                        if(pixelColorStatus[nborI][nborJ] != PixelStatusEnum::colored) continue;
                        interPixels.emplace_back(nborI,nborJ);
                        lastVisit[nborI][nborJ] = Time;
                        pixelColorStatus[nborI][nborJ] = PixelStatusEnum::intersection;
                    
                    }
                }
                intersectionsList.push_back(Intersection(interPixels));
            } else if(pixelColorStatus[a][b] == PixelStatusEnum::notcolored){
                //std::cout<<"SET NEW COLOR "<<a<<" "<<b<<std::endl;
                pixelColorStatus[a][b] = PixelStatusEnum::newcolor;
            }        
    return intersectionsList;
}
void ImageTexture::markMinABCut(std::pair<int, int> S, std::pair<int, int> T, const ImageTexture::Intersection &inter){
    static std::vector<std::vector<bool>> inSubgraph(imgHeight + 1, std::vector<bool>(imgWidht + 1));
    static std::vector<std::vector<std::array<int, 4>>> edgesCosts(imgHeight + 1, std::vector<std::array<int, 4>>(imgWidht + 1));
    /*mark cells in dual of intersection and mark edges costs*/{
        std::vector<std::pair<int, int>> inDual;
        for(auto [i,j] : inter.interPixels)
            for(auto [di, dj] : primalToDual)
                if(insidePrimal(i + di, j + dj)){
                    inSubgraph[i + di][j + dj] = true;
                    inDual.emplace_back(i + di, j  + dj);
                }
        
        
    }
    /*mark edges costs*/{

    }
    /*find min cut (min s-t path)*/{

    }
    /*unmark cells in dual of intersection*/{
        for(auto [i,j] : inter.interPixels)
            for(auto [di, dj] : primalToDual)
                if(insidePrimal(i + di, j + dj))
                    inSubgraph[i + di][j + dj] = false;
    }
}