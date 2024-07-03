#include <png++/png.hpp>
#include <algorithm>
#include <math.h>
#include <utility>
#include <array>
#include <queue>
#include <tuple>
#include <map>
#include <random>
#include <chrono>
#include <cassert>

#include <unistd.h>  //just for debug
#include <cstdlib> // just for debug
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
    png::image<png::rgb_pixel> outputImg;
    const int imgWidth;
    const int imgHeight;
    std::vector<std::vector<PixelStatusEnum>> pixelColorStatus;
    static constexpr long double inftyCost = 2000000;
    static constexpr std::array<std::pair<int, int>, 4> directions = {{
        {-1, 0},    //    |0|
        { 0,-1},    //  |1|x|3|
        { 1, 0},    //    |2|
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
    static int revDir(int i);
    static long double pow2(long double x);
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
    bool inImgBorder(int i, int j, int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);    
    bool insidePrimal(int i, int j);  
    bool insideDual(int i, int j);
    bool insideImg(int i, int j, const png::image<png::rgb_pixel> &img);
    class Intersection{
        public:
            // counter clockwise, starting with upper neighbor
            std::vector<std::pair<int, int>> interPixels;
            Intersection(const std::vector<std::pair<int,int>> &pixels = {}) : interPixels(pixels){}
    };
    //int heightOffset, int widthOffset for debug, will change later
    std::pair<std::pair<int, int>, std::pair<int, int> > findSTInIntersection(Intersection &inter, int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    std::vector<Intersection> findIntersections(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    void markMinABCut(std::pair<int, int> S, std::pair<int, int> T, const ImageTexture::Intersection &inter, int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    void markIntersectionRed(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg); //just to debug
};

/*
Constructors
*/
ImageTexture::ImageTexture(const png::image<png::rgb_pixel> & _img) 
    : outputImg(_img), 
    imgWidth(_img.get_width()),
    imgHeight(_img.get_height()),
    pixelColorStatus(_img.get_width(), std::vector<PixelStatusEnum>(_img.get_height(), PixelStatusEnum::notcolored)){
}
ImageTexture::ImageTexture(int width, int height) 
    : outputImg(width, height), 
    imgWidth(width),
    imgHeight(height), 
    pixelColorStatus(width, std::vector<PixelStatusEnum>(height, PixelStatusEnum::notcolored)){    
}

/*
Public Functions
*/
void ImageTexture::render(const std::string &file_name){
    outputImg.write(file_name);
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
    //static std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    static std::mt19937_64 rng(12275994992249);
    static std::uniform_int_distribution<int> nextHeight(0, imgHeight-1);
    static std::uniform_int_distribution<int> nextWidth(0, imgWidth-1);
    return {nextHeight(rng), nextWidth(rng)};
    static int heightOffset = 0;
    static int widthOffset = -24;
    widthOffset += 24;
    if(widthOffset >= this->imgWidth){
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
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
            if(this->pixelColorStatus[a][b] != PixelStatusEnum::notcolored){
                return false;
            }
        }
    return true;    
}
void ImageTexture::copyFirstPatch(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    std::cout<<"Copy First Patch"<<std::endl;
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
            this->pixelColorStatus[a][b] = PixelStatusEnum::newcolor;
        }  
    copyPixelsNewColor(heightOffset, widthOffset, inputImg);
}
bool ImageTexture::stPlanarGraph(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    {//upper edge
        int upperEdgeHeight = heightOffset;
        for(int j = 0; j < (int) inputImg.get_width() && j + widthOffset < this->imgWidth; j++){
            if(this->pixelColorStatus[upperEdgeHeight][j + widthOffset] == this->PixelStatusEnum::notcolored)
                return true;
        }
        std::cout<<"upper edge is covered "<<upperEdgeHeight<<std::endl;
    }
    {//lower edge
        int lowerEdgeHeight = std::min<int>(heightOffset + inputImg.get_height() - 1 , this->imgHeight - 1);
        for(int j = 0; j < (int) inputImg.get_width() && j + widthOffset < this->imgWidth; j++){
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
        int rightEdgeWidth = std::min<int>(widthOffset + inputImg.get_width() - 1, this->imgWidth - 1);
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
        auto [S, T] = findSTInIntersection(inter, heightOffset, widthOffset, inputImg);
        markMinABCut(S, T, inter, heightOffset, widthOffset, inputImg);
    }
    // for(auto &inter : intersections){
    //     for(auto [i,j] : inter.interPixels)
    //         pixelColorStatus[i][j] = PixelStatusEnum::colored;
    // }
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
long double ImageTexture::pow2(long double x){
    return x * x;
}
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
int ImageTexture::revDir(int i){
    return (i + directions.size() / 2) % directions.size();
}
// Auxiliar Nonstatic Functions
void ImageTexture::copyPixelsNewColor(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++)
            if(pixelColorStatus[a][b] == PixelStatusEnum::newcolor){
                outputImg[a][b] = png::rgb_pixel(0,0,155);
            }
    render("../output/areia_da_praia.png");
    usleep(800000);
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++)
            if(pixelColorStatus[a][b] == PixelStatusEnum::newcolor){
                outputImg[a][b] = inputImg[i][j];
                pixelColorStatus[a][b] = PixelStatusEnum::colored;
            }
}
bool ImageTexture::inImgBorder(int i, int j, int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    return i == heightOffset || i == std::min<int>(imgHeight - 1, heightOffset + inputImg.get_height() - 1) 
        || j == widthOffset  || j == std::min<int>(imgWidth - 1, widthOffset + inputImg.get_width() - 1);
}
bool ImageTexture::insidePrimal(int i, int j){
    return 0 <= i && i < imgHeight && 0 <= j && j < imgWidth;
}
bool ImageTexture::insideDual(int i, int j){
    return 0 <= i && i < imgHeight + 1 && 0 <= j && j < imgWidth + 1;
}

bool ImageTexture::insideImg(int i, int j, const png::image<png::rgb_pixel> &img){
    return 0 <= i && i < int(img.get_height()) && 0 <= j && j < int(img.get_width());
}
std::pair<std::pair<int, int>, std::pair<int, int> > ImageTexture::findSTInIntersection(ImageTexture::Intersection &inter, int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    std::pair<int, int> S = {-1,-1}, T = {-1,-1};
    for(const auto &[i, j] : inter.interPixels){
        for(int d = 0; d < (int) directions.size(); d++){
            int neiI = i + directions[d].first;
            int neiJ = j + directions[d].second;
            if(!insidePrimal(neiI, neiJ) || pixelColorStatus[neiI][neiJ] != PixelStatusEnum::newcolor)
                continue;
            { //S
                int nextI = i + directions[prevDir(d)].first;
                int nextJ = j + directions[prevDir(d)].second;
                if(!insidePrimal(nextI, nextJ)){
                    S = {i + primalToDual[prevDir(d)].first, j + primalToDual[prevDir(d)].second};
                }else if(pixelColorStatus[nextI][nextJ] != PixelStatusEnum::newcolor && pixelColorStatus[nextI][nextJ] != PixelStatusEnum::intersection){
                    S = {i + primalToDual[prevDir(d)].first, j + primalToDual[prevDir(d)].second};
                }

            }
            { //T
                    int nextI = i + directions[nextDir(d)].first;
                    int nextJ = j + directions[nextDir(d)].second;
                    if(!insidePrimal(nextI, nextJ)){
                        T = {i + primalToDual[d].first, j + primalToDual[d].second};
                    }else if(pixelColorStatus[nextI][nextJ] != PixelStatusEnum::newcolor && pixelColorStatus[nextI][nextJ] != PixelStatusEnum::intersection){
                        T = {i + primalToDual[d].first, j + primalToDual[d].second};
                    }
            }
        }
    }
    if((S) == (std::pair<int, int>{-1,-1}) || (T) == (std::pair<int, int>{-1,-1})){
        for(auto [x,y] : inter.interPixels){
            outputImg[x][y] = png::rgb_pixel(0,155,0);
        }
        render("../output/areia_da_praia.png");
    }
    assert(("findSTInIntersection should always find S", (S) != (std::pair<int, int>{-1,-1})));
    assert(("findSTInIntersection should always find T", (T) != (std::pair<int, int>{-1,-1})));
    std::cout<<" S is "<<S.first<<" "<<S.second<<std::endl;
    std::cout<<" T is "<<T.first<<" "<<T.second<<std::endl;
    return {S, T};
}
std::vector<ImageTexture::Intersection> ImageTexture::findIntersections(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    static std::vector<std::vector<int>> lastVisit(imgHeight, std::vector<int>(imgWidth, 0));
    static int Time = 0;
    std::vector<Intersection> intersectionsList;
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++)
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
void ImageTexture::markMinABCut(std::pair<int, int> S, std::pair<int, int> T, const ImageTexture::Intersection &inter, int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    static std::vector<std::vector<bool>> inSubgraph(imgHeight + 1, std::vector<bool>(imgWidth + 1));
    static std::vector<std::vector<std::array<long double, 4>>> edgesCosts(imgHeight + 1, std::vector<std::array<long double, 4>>(imgWidth + 1));
    /*mark cells in dual of intersection and mark edges costs*/{
        std::vector<std::pair<int, int>> inDual;
        for(auto [i,j] : inter.interPixels)
            for(auto [di, dj] : primalToDual)
                if(insideDual(i + di, j + dj)){
                    inSubgraph[i + di][j + dj] = true;
                    inDual.emplace_back(i + di, j  + dj);
                }    
        sort(inDual.begin(), inDual.end());
        inDual.resize(unique(inDual.begin(), inDual.end()) - inDual.begin());
        for(auto [i,j] : inDual)
            for(int d = 0; d < (int) directions.size(); d++){
                int nextI = i + directions[d].first;
                int nextJ = j + directions[d].second;
                if(insideDual(nextI, nextJ) && inSubgraph[nextI][nextJ]){
                    int iA = i + dualToPrimal[d].first, jA = j + dualToPrimal[d].second;
                    int iB = i + dualToPrimal[prevDir(d)].first, jB = j + dualToPrimal[prevDir(d)].second;
                    if(insidePrimal(iA, jA) && pixelColorStatus[iA][jA] == PixelStatusEnum::intersection && insidePrimal(iB, jB) && pixelColorStatus[iB][jB] == PixelStatusEnum::intersection)
                        edgesCosts[i][j][d] = calcCost(outputImg[iA][jA], outputImg[iB][jB], inputImg[iA - heightOffset][jA - widthOffset], inputImg[iB - heightOffset][jB - widthOffset]);
                    else
                        edgesCosts[i][j][d] = inftyCost;
                }
            }
    }
    
    static std::vector<std::vector<long double>> dist(imgHeight + 1, std::vector<long double>(imgWidth + 1));
    static std::vector<std::vector<bool>> vis(imgHeight + 1, std::vector<bool>(imgWidth + 1));
    static std::vector<std::vector<int>> parent(imgHeight + 1, std::vector<int>(imgWidth + 1, -1));
    static std::vector<std::vector<std::array<bool, 4>>> validEdge(imgHeight + 1, std::vector<std::array<bool, 4>>(imgWidth + 1, {true,true,true,true}));
    /*find min cut (min s-t path)*/{
        using qtype = std::tuple<long double, int, int>;
        std::priority_queue<qtype, std::vector<qtype>, std::greater<qtype>> Q;
        parent[S.first][S.second] = -2;
        dist[S.first][S.second] = 0;
        Q.emplace(0, S.first, S.second);
        while(!Q.empty()){
            long double pathCost;
            int i, j;
            std::tie(pathCost, i, j) = Q.top();
            Q.pop();
            if(vis[i][j])
                continue;
            vis[i][j] = true;
            if(T == std::pair<int, int>{i,j})
                break;
            for(int d = 0; d < int(directions.size()); d++){
                int nextI = i + directions[d].first, nextJ = j + directions[d].second;
                if(insideDual(nextI, nextJ) && inSubgraph[nextI][nextJ] && !vis[nextI][nextJ]){
                    long double curCost = edgesCosts[i][j][d];
                    if(parent[nextI][nextJ] == -1 || pathCost + curCost < dist[nextI][nextJ]){
                        parent[nextI][nextJ] = revDir(d);
                        dist[nextI][nextJ] = pathCost + curCost; // avoid overflow
                        Q.emplace(dist[nextI][nextJ], nextI, nextJ);
                    }
                }
            }
        }
        if(!vis[T.first][T.second]){        
            for(auto [x,y] : inter.interPixels){
                outputImg[x][y] = png::rgb_pixel(155,0,0);
            }
            render("../output/areia_da_praia.png");
        }
        assert(("T should always be visited, intersection is connected", (vis[T.first][T.second])));
        { // mark ST path
            std::cout<<"ST path"<<std::endl;
            int curI = T.first, curJ = T.second;
            while(std::make_pair(curI, curJ) != S){
                std::cout<<std::fixed<<std::setprecision(10)<<"\t"<<curI<<" "<<curJ<<" -- dir "<<parent[curI][curJ]<<" distance "<<dist[curI][curJ]<<std::endl;
                int d = parent[curI][curJ];
                assert(0 <= d && d < int(directions.size()));
                validEdge[curI][curJ][d] = false;
                std::tie(curI, curJ) = std::make_pair(curI + directions[d].first, curJ + directions[d].second);
                d = revDir(d);
                validEdge[curI][curJ][d] = false;
            }
        }
    }
    /*mark left and right of min cut*/{
        std::cout<<"Mark left of min cut"<<std::endl;
        int curI = T.first, curJ = T.second;
        
        while(std::make_pair(curI, curJ) != S){
            std::cout<<std::fixed<<std::setprecision(10)<<"\t"<<curI<<" "<<curJ<<" -- dir "<<parent[curI][curJ]<<" distance "<<dist[curI][curJ]<<std::endl;
            int d = parent[curI][curJ];
            assert(0 <= d && d < int(directions.size()));
            std::tie(curI, curJ) = std::make_pair(curI + directions[d].first, curJ + directions[d].second);
            d = revDir(d);
            int firstI, firstJ;
            std::tie(firstI, firstJ) = std::make_pair(curI + dualToPrimal[d].first, curJ + dualToPrimal[d].second);
            if(insidePrimal(firstI, firstJ) && pixelColorStatus[firstI][firstJ] == PixelStatusEnum::intersection){
                std::cout<<"Start BFS from "<<firstI<<" "<<firstJ<<std::endl;
                std::cout<<"  Because "<<curI<<" "<<curJ<<" dir "<<d<<" "<<revDir(d)<<std::endl;
                std::vector<std::pair<int, int>> q = {{firstI, firstJ}};
                pixelColorStatus[firstI][firstJ] = PixelStatusEnum::colored;
                for(int front = 0; front < int(q.size()); front++){
                    int fI = q[front].first, fJ = q[front].second;
                    std::cout<<"\tfront "<<fI<<" "<<fJ<<std::endl;
                    for(int d = 0; d < (int) directions.size(); d++){
                        int nxtI = fI + directions[d].first;
                        int nxtJ = fJ + directions[d].second;
                        if(!insidePrimal(nxtI, nxtJ) || pixelColorStatus[nxtI][nxtJ] != PixelStatusEnum::intersection)
                            continue;
                        int dualI = fI + primalToDual[d].first;
                        int dualJ = fJ + primalToDual[d].second;
                        assert(insideDual(dualI, dualJ));
                        std::cout<<"\t   Test "<<nxtI<<" "<<nxtJ<<" valid dual "<<dualI<<" "<<dualJ<<" "<<prevDir(d)<<std::endl;
                        if(validEdge[dualI][dualJ][prevDir(d)]){
                            std::cout<<"\t   To "<<nxtI<<" "<<nxtJ<<" valid dual "<<dualI<<" "<<dualJ<<" "<<prevDir(d)<<std::endl;
                            pixelColorStatus[nxtI][nxtJ] = PixelStatusEnum::colored;
                            q.emplace_back(nxtI, nxtJ);
                        }
                    }
                }
            }  
        }
        
    
        for(auto [i, j] : inter.interPixels) //mark right of min cut
            if(pixelColorStatus[i][j] == PixelStatusEnum::intersection){
                pixelColorStatus[i][j] = PixelStatusEnum::newcolor;
            }
    }
    /*unmark cells in dual of intersection*/{
        { //unmark ST path
            int curI = T.first, curJ = T.second;
            while(std::make_pair(curI, curJ) != S){
                //std::cout<<"\t"<<curI<<" "<<curJ<<" -- dir "<<parent[curI][curJ]<<std::endl;
                int d = parent[curI][curJ];
                assert(0 <= d && d < int(directions.size()));
                validEdge[curI][curJ][d] = true;
                std::tie(curI, curJ) = std::make_pair(curI + directions[d].first, curJ + directions[d].second);
                d = revDir(d);
                validEdge[curI][curJ][d] = true;
            }
        }
        for(auto [i,j] : inter.interPixels)
            for(auto [di, dj] : primalToDual)
                if(insidePrimal(i + di, j + dj)){
                    inSubgraph[i + di][j + dj] = false;
                    parent[i + di][j + dj] = -1;
                    vis[i + di][j + dj] = false;
                }
    }
}
void ImageTexture::markIntersectionRed(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    std::cout<<"MARK RED\n";
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++)
            outputImg[a][b] = png::rgb_pixel(155,0,0);
    render("../output/areia_da_praia.png");    
}