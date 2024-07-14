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
#include <iomanip> // just for debug
#include <string>
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
    static constexpr long double inftyCost = 10000000;
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
    
    std::pair<int, int> matching(const png::image<png::rgb_pixel> &inputImg);
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
    std::pair<std::pair<int, int>, std::pair<int, int> > findSTInIntersectionCase1(Intersection &inter, int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    std::vector<Intersection> findIntersections(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    void markMinABCut(std::pair<int, int> S, std::pair<int, int> T, const ImageTexture::Intersection &inter, int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    //MarkMinABCut auxiliar functions
    std::vector<std::pair<int, int>> markIntersectionCellsInDual(const ImageTexture::Intersection &inter, std::vector<std::vector<bool>> &inSubgraph);
    void markIntersectionEdgeCostsInDual(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg, const std::vector<std::pair<int, int>> &inDual, const std::vector<std::vector<bool>> &inSubgraph, std::vector<std::vector<std::array<long double, 4>>> &edgesCosts);
    std::vector<std::pair<int,int>> findSTPath(const std::vector<std::pair<int,int>> &S, const std::vector<std::pair<int,int>> &T, const std::vector<std::vector<bool>> &inSubgraph, const std::vector<std::vector<std::array<long double, 4>>> &edgesCosts, std::vector<std::vector<long double>> &dist, std::vector<std::vector<bool>> &vis, std::vector<std::vector<int>> &parent);
    void markLeftOfMinCut(std::vector<std::pair<int, int>> &cut, std::vector<std::vector<int>> &parent, const std::vector<std::vector<std::array<bool, 4>>> &validEdge);
    //Case 2 auxiliar functions
    std::vector<std::pair<int, int>> dualBorder(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    std::vector<std::pair<int, int>> findSCase2(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg); 
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
    const auto [heightOffset, widthOffset] = matching(inputImg);
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
std::pair<int, int> ImageTexture::matching(const png::image<png::rgb_pixel> &inputImg){
    static const uint64_t rngSeed = 36609795014301;
    //static const uint64_t rngSeed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::cout<<"Seed is "<<rngSeed<<std::endl;
    static std::mt19937_64 rng(rngSeed);
    static std::uniform_int_distribution<int> nextHeight(-inputImg.get_height() + 1, imgHeight-1);
    static std::uniform_int_distribution<int> nextWidth(-inputImg.get_width() + 1, imgWidth-1);
    return {nextHeight(rng), nextWidth(rng)};
}
bool ImageTexture::isFirstPatch(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
            if(a < 0 || b < 0)
                continue;
            if(this->pixelColorStatus[a][b] != PixelStatusEnum::notcolored){
                return false;
            }
        }
    return true;    
}
void ImageTexture::copyFirstPatch(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    //std::cout<<"Copy First Patch"<<std::endl;
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
            if(a < 0 || b < 0)
                continue;
            this->pixelColorStatus[a][b] = PixelStatusEnum::newcolor;
        }  
    copyPixelsNewColor(heightOffset, widthOffset, inputImg);
}
bool ImageTexture::stPlanarGraph(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    {//upper edge
        int upperEdgeHeight = std::max(0, heightOffset);
        for(int j = 0; j < (int) inputImg.get_width() && j + widthOffset < this->imgWidth; j++){
            if(j + widthOffset < 0)
                continue;
            if(this->pixelColorStatus[upperEdgeHeight][j + widthOffset] == this->PixelStatusEnum::notcolored)
                return true;
        }
        //std::cout<<"upper edge is covered "<<upperEdgeHeight<<std::endl;
    }
    {//left edge
        int leftEdgeWidth = std::max(0, widthOffset);
        for(int i = 0; i < (int) inputImg.get_height() && i + heightOffset < this->imgHeight; i++){
            if(i + heightOffset < 0)
                continue;
            if(this->pixelColorStatus[i + heightOffset][leftEdgeWidth] == this->PixelStatusEnum::notcolored)
                return true;
        }
        //std::cout<<"left edge is covered "<<leftEdgeWidth<<std::endl;
    }
    {//lower edge
        int lowerEdgeHeight = std::min<int>(heightOffset + inputImg.get_height() - 1 , this->imgHeight - 1);
        for(int j = 0; j < (int) inputImg.get_width() && j + widthOffset < this->imgWidth; j++){
            if(j + widthOffset < 0)
                continue;
            if(this->pixelColorStatus[lowerEdgeHeight][j + widthOffset] == this->PixelStatusEnum::notcolored)
                return true;
        }
        //std::cout<<"lower edge is covered "<<lowerEdgeHeight<<std::endl;
    }
    {//right edge
        int rightEdgeWidth = std::min<int>(widthOffset + inputImg.get_width() - 1, this->imgWidth - 1);
        for(int i = 0; i < (int) inputImg.get_height() && i + heightOffset < this->imgHeight; i++){
            if(i + heightOffset < 0)
                continue;
            if(this->pixelColorStatus[i + heightOffset][rightEdgeWidth] == this->PixelStatusEnum::notcolored)
                return true;
        }
        //std::cout<<"right edge is covered "<<rightEdgeWidth<<std::endl;
    }
    return false;
}
void ImageTexture::blendingCase1(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    std::cout<<"Case 1"<<std::endl;
    std::vector<Intersection> intersections = findIntersections(heightOffset, widthOffset, inputImg);
    //std::cout<<"numer of intersections "<<intersections.size()<<std::endl;
    for(auto &inter : intersections){
        //std::cout<<"intersection size "<<inter.interPixels.size()<<std::endl;
        auto [S, T] = findSTInIntersectionCase1(inter, heightOffset, widthOffset, inputImg);
        markMinABCut(S, T, inter, heightOffset, widthOffset, inputImg);
    }
    // for(auto &inter : intersections){
    //     for(auto [i,j] : inter.interPixels)
    //         pixelColorStatus[i][j] = PixelStatusEnum::colored;
    // }
    copyPixelsNewColor(heightOffset, widthOffset, inputImg);

    /*sanity test*/{
        for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
            for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
                if(a < 0 || b < 0)
                    continue;
                assert(("All pixels should be colored", pixelColorStatus[a][b] == PixelStatusEnum::colored));
            }
    }
}
void ImageTexture::blendingCase2(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    std::cout<<"Case 2"<<std::endl;
    static std::vector<std::vector<bool>> inSubgraph(imgHeight + 1, std::vector<bool>(imgWidth + 1));
    static std::vector<std::vector<std::array<long double, 4>>> edgesCosts(imgHeight + 1, std::vector<std::array<long double, 4>>(imgWidth + 1));    
    static std::vector<std::vector<long double>> dist(imgHeight + 1, std::vector<long double>(imgWidth + 1));
    static std::vector<std::vector<bool>> vis(imgHeight + 1, std::vector<bool>(imgWidth + 1));
    static std::vector<std::vector<int>> parent(imgHeight + 1, std::vector<int>(imgWidth + 1, -1));
    static std::vector<std::vector<std::array<bool, 4>>> validEdge(imgHeight + 1, std::vector<std::array<bool, 4>>(imgWidth + 1, {true,true,true,true}));

    Intersection intersection;
    /*find intersection*/
    auto intersections = findIntersections(heightOffset, widthOffset, inputImg);
    assert(intersections.size() == 1);
    intersection = intersections[0];
    
    /*mark cells in dual of intersection and mark edges costs*/
    std::vector<std::pair<int, int>> inDual = markIntersectionCellsInDual(intersection, inSubgraph);
    markIntersectionEdgeCostsInDual(heightOffset, widthOffset, inputImg, inDual, inSubgraph, edgesCosts);

    /*find ST path*/
    auto S = findSCase2(heightOffset, widthOffset, inputImg);
    auto T = dualBorder(heightOffset, widthOffset, inputImg);
    auto tsPath = findSTPath(S, T, inSubgraph, edgesCosts, dist, vis, parent);

    /*mark ST path*/{
        //std::cout<<"ST path"<<std::endl;
        int lastD = -1;
        for(auto [curI, curJ] : tsPath){
            if(lastD >= 0){
                validEdge[curI][curJ][lastD] = false;
            }
            int d = parent[curI][curJ];
            if(d >= 0){
                //std::cout<<std::fixed<<std::setprecision(10)<<"\t"<<curI<<" "<<curJ<<" -- dir "<<parent[curI][curJ]<<" distance "<<dist[curI][curJ]<<std::endl;
                validEdge[curI][curJ][d] = false;
                lastD = revDir(d);
            }
        }
    }    

    /*unmark cells in dual of intersection*/{
        for(auto [i,j] : intersection.interPixels)
            for(auto [di, dj] : primalToDual)
                if(insideDual(i + di, j + dj)){
                    inSubgraph[i + di][j + dj] = false;
                    parent[i + di][j + dj] = -1;
                    vis[i + di][j + dj] = false;
                }
    }
    //TO DO

    /*unmark cells in dual of intersection*/{
    { //unmark ST path
        int lastD = -1;
        for(auto [curI, curJ] : tsPath){
            if(lastD >= 0){
                validEdge[curI][curJ][lastD] = true;
            }
            int d = parent[curI][curJ];
            if(d >= 0){
                validEdge[curI][curJ][d] = true;
                lastD = revDir(d);
            }
        }
    }
    for(auto [i,j] : intersection.interPixels)
        for(auto [di, dj] : primalToDual)
            if(insideDual(i + di, j + dj)){
                inSubgraph[i + di][j + dj] = false;
                parent[i + di][j + dj] = -1;
                vis[i + di][j + dj] = false;
            }
    }


    /*undoing intersections*/{ //REMOVE LATER
        for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
            for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
                if(a < 0 || b < 0)
                    continue;
                if(pixelColorStatus[a][b] == PixelStatusEnum::intersection)
                    pixelColorStatus[a][b] = PixelStatusEnum::colored;
                else 
                    pixelColorStatus[a][b] = PixelStatusEnum::notcolored;
            }
    }
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
    int cnt = 0;
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
            if(a < 0 || b < 0)
                continue;
            if(pixelColorStatus[a][b] == PixelStatusEnum::newcolor){
                cnt++;
                outputImg[a][b] = png::rgb_pixel(0,0,155);
            }
        }
    std::cout<<"NEW COLOR "<<cnt<<std::endl;
    render("../output/output.png");
    usleep(400000);
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
            if(a < 0 || b < 0)
                continue;
            if(pixelColorStatus[a][b] == PixelStatusEnum::newcolor){
                outputImg[a][b] = inputImg[i][j];
                pixelColorStatus[a][b] = PixelStatusEnum::colored;
            }
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
std::pair<std::pair<int, int>, std::pair<int, int> > ImageTexture::findSTInIntersectionCase1(ImageTexture::Intersection &inter, int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
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
        usleep(400000);
        //std::cout<<"OFFSET height "<<heightOffset<<" width "<<widthOffset<<std::endl;
        for(auto [x,y] : inter.interPixels){
            outputImg[x][y] = png::rgb_pixel(0,155,0);
        }
        render("../output/output.png");
        usleep(4000000);

    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
            if(a < 0 || b < 0)
                continue;
            if(pixelColorStatus[a][b] == PixelStatusEnum::newcolor)
                outputImg[a][b] = png::rgb_pixel(0,0,155);
            
        }
        //render("../output/output.png");
    }
    assert(("findSTInIntersectionCase1 should always find S", (S) != (std::pair<int, int>{-1,-1})));
    std::cout<<" S is "<<S.first<<" "<<S.second<<std::endl;
    assert(("findSTInIntersectionCase1 should always find T", (T) != (std::pair<int, int>{-1,-1})));
    std::cout<<" T is "<<T.first<<" "<<T.second<<std::endl;
    assert(("findSTInIntersectionCase1 should find different S and T", (S) != (T)));
    return {S, T};
}
std::vector<ImageTexture::Intersection> ImageTexture::findIntersections(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    static std::vector<std::vector<int>> lastVisit(imgHeight, std::vector<int>(imgWidth, 0));
    static int Time = 0;
    std::vector<Intersection> intersectionsList;
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
            if(a < 0 || b < 0)
                continue;
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
                pixelColorStatus[a][b] = PixelStatusEnum::newcolor;
            }      
        }  
    return intersectionsList;
}
void ImageTexture::markMinABCut(std::pair<int, int> S, std::pair<int, int> T, const ImageTexture::Intersection &inter, int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    static std::vector<std::vector<bool>> inSubgraph(imgHeight + 1, std::vector<bool>(imgWidth + 1));
    static std::vector<std::vector<std::array<long double, 4>>> edgesCosts(imgHeight + 1, std::vector<std::array<long double, 4>>(imgWidth + 1));    
    static std::vector<std::vector<long double>> dist(imgHeight + 1, std::vector<long double>(imgWidth + 1));
    static std::vector<std::vector<bool>> vis(imgHeight + 1, std::vector<bool>(imgWidth + 1));
    static std::vector<std::vector<int>> parent(imgHeight + 1, std::vector<int>(imgWidth + 1, -1));
    static std::vector<std::vector<std::array<bool, 4>>> validEdge(imgHeight + 1, std::vector<std::array<bool, 4>>(imgWidth + 1, {true,true,true,true}));

    /*mark cells in dual of intersection and mark edges costs*/
    std::vector<std::pair<int, int>> inDual = markIntersectionCellsInDual(inter, inSubgraph);
    markIntersectionEdgeCostsInDual(heightOffset, widthOffset, inputImg, inDual, inSubgraph, edgesCosts);

    /*find min cut (min s-t path)*/
    auto tsPath = findSTPath({S}, {T}, inSubgraph, edgesCosts, dist, vis, parent);
    
    /*mark edges on the path*/
    assert(("T should always be visited, intersection is connected", (vis[T.first][T.second])));
    
    /*mark ST path*/{
        //std::cout<<"ST path"<<std::endl;
        int lastD = -1;
        for(auto [curI, curJ] : tsPath){
            if(lastD >= 0){
                validEdge[curI][curJ][lastD] = false;
            }
            int d = parent[curI][curJ];
            if(d >= 0){
                //std::cout<<std::fixed<<std::setprecision(10)<<"\t"<<curI<<" "<<curJ<<" -- dir "<<parent[curI][curJ]<<" distance "<<dist[curI][curJ]<<std::endl;
                validEdge[curI][curJ][d] = false;
                lastD = revDir(d);
            }
        }
    }    
    
    /*mark left and right of min cut*/
    //std::cout<<"Mark left of min cut"<<std::endl;        
    markLeftOfMinCut(tsPath, parent, validEdge);
    /*mark right of min cut*/{    
        for(auto [i, j] : inter.interPixels)
            if(pixelColorStatus[i][j] == PixelStatusEnum::intersection){
                pixelColorStatus[i][j] = PixelStatusEnum::newcolor;
            }
    }
    
    /*unmark cells in dual of intersection*/{
        { //unmark ST path
            int lastD = -1;
            for(auto [curI, curJ] : tsPath){
                if(lastD >= 0){
                    validEdge[curI][curJ][lastD] = true;
                }
                int d = parent[curI][curJ];
                if(d >= 0){
                    validEdge[curI][curJ][d] = true;
                    lastD = revDir(d);
                }
            }
        }
        for(auto [i,j] : inter.interPixels)
            for(auto [di, dj] : primalToDual)
                if(insideDual(i + di, j + dj)){
                    inSubgraph[i + di][j + dj] = false;
                    parent[i + di][j + dj] = -1;
                    vis[i + di][j + dj] = false;
                }
    }
}
std::vector<std::pair<int, int>> ImageTexture::markIntersectionCellsInDual(const Intersection &inter, std::vector<std::vector<bool>> &inSubgraph){
    std::vector<std::pair<int, int>> inDual;
    for(auto [i,j] : inter.interPixels)
        for(auto [di, dj] : primalToDual)
            if(insideDual(i + di, j + dj)){
                inSubgraph[i + di][j + dj] = true;
                inDual.emplace_back(i + di, j + dj);
            }    
    sort(inDual.begin(), inDual.end());
    inDual.resize(unique(inDual.begin(), inDual.end()) - inDual.begin());
    return inDual;
}
void ImageTexture::markIntersectionEdgeCostsInDual(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg, const std::vector<std::pair<int, int>> &inDual, const std::vector<std::vector<bool>> &inSubgraph, std::vector<std::vector<std::array<long double, 4>>> &edgesCosts){
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

std::vector<std::pair<int,int>> ImageTexture::findSTPath(const std::vector<std::pair<int,int>> &S, const std::vector<std::pair<int,int>> &T, const std::vector<std::vector<bool>> &inSubgraph, const std::vector<std::vector<std::array<long double, 4>>> &edgesCosts, std::vector<std::vector<long double>> &dist, std::vector<std::vector<bool>> &vis, std::vector<std::vector<int>> &parent){
    static std::vector<std::vector<bool>> isT(imgHeight + 1, std::vector<bool>(imgWidth+1, false));
    static std::vector<std::vector<bool>> isS(imgHeight + 1, std::vector<bool>(imgWidth+1, false));
    for(auto [h, w] : T){
        isT[h][w] = true;
    }
    using qtype = std::tuple<long double, int, int>;
    std::priority_queue<qtype, std::vector<qtype>, std::greater<qtype>> Q;
    for(auto [h, w] : S){
        isS[h][w] = true;
        parent[h][w] = -2;
        dist[h][w] = 0;
        Q.emplace(0, h, w);
    }
    //std::cout<<"START DIJKSTRA"<<std::endl;
    std::vector<std::pair<int, int>> path;
    while(!Q.empty()){
        long double pathCost;
        int i, j;
        std::tie(pathCost, i, j) = Q.top();
        Q.pop();
        //std::cout<<"Test "<<i<<" "<<j<<std::endl;
        assert(i < vis.size());
        assert(j < vis[i].size());
        if(vis[i][j])
            continue;
        //std::cout<<"Dijkstra "<<i<<" "<<j<<std::endl;
        vis[i][j] = true;
        if(isT[i][j]){
            path = {{i,j}};
            break;
        }
        for(int d = 0; d < int(directions.size()); d++){
            int nextI = i + directions[d].first, nextJ = j + directions[d].second;
            if(insideDual(nextI, nextJ) && inSubgraph[nextI][nextJ] && !vis[nextI][nextJ]){
                long double curCost = edgesCosts[i][j][d];
                if(isS[i][j] && isT[nextI][nextJ]) //avoid direct empty cut
                    continue;
                if(parent[nextI][nextJ] == -1 || pathCost + curCost < dist[nextI][nextJ]){
                    parent[nextI][nextJ] = revDir(d);
                    dist[nextI][nextJ] = pathCost + curCost; // avoid overflow
                    Q.emplace(dist[nextI][nextJ], nextI, nextJ);
                }
            }
        }
    }
    // only path is direct path between S and T
    if(!path.empty()){
        //std::cout<<"PATH IS EMPTY"<<std::endl;
        bool ok = false;
        for(auto [i, j] : S){
            for(int d = 0; d < int(directions.size()); d++){
                int nextI = i + directions[d].first, nextJ = j + directions[d].second;
                if(insideDual(nextI, nextJ) && inSubgraph[nextI][nextJ] && !vis[nextI][nextJ] && isT[nextI][nextJ]){
                    parent[nextI][nextJ] = revDir(d);
                    path = {{nextI, nextJ}};
                    break;
                }
            }
            if(!path.empty())
                break;
        }
    }
    assert(("path is empty!", !path.empty()));
    int curI, curJ;
    std::tie(curI, curJ) = path[0];
    while(parent[curI][curJ] != -2){
        //std::cout<<std::fixed<<std::setprecision(10)<<"\t"<<curI<<" "<<curJ<<" -- dir "<<parent[curI][curJ]<<" distance "<<dist[curI][curJ]<<std::endl;
        int d = parent[curI][curJ];
        assert(0 <= d && d < int(directions.size()));
        std::tie(curI, curJ) = std::make_pair(curI + directions[d].first, curJ + directions[d].second);
        path.emplace_back(curI, curJ);
    }
    for(auto [h, w] : T){
        isT[h][w] = false;
    }
    for(auto [h, w] : S){
        isS[h][w] = false;
    }
    return path;
}   

void ImageTexture::markLeftOfMinCut(std::vector<std::pair<int, int>> &cut, std::vector<std::vector<int>> &parent, const std::vector<std::vector<std::array<bool, 4>>> &validEdge){
    for(auto [i, j] : cut){
        int d = parent[i][j];
        if(d < 0)
            break;
        assert(0 <= d && d < int(directions.size()));
        //std::cout<<std::fixed<<std::setprecision(10)<<"\t"<<i<<" "<<j<<" -- dir "<<parent[i][j]<<std::endl;
        auto [curI, curJ] = std::make_pair(i + directions[d].first, j + directions[d].second);
        d = revDir(d);
        int firstI, firstJ;
        std::tie(firstI, firstJ) = std::make_pair(curI + dualToPrimal[d].first, curJ + dualToPrimal[d].second);
        //BFS Marking left
        if(insidePrimal(firstI, firstJ) && pixelColorStatus[firstI][firstJ] == PixelStatusEnum::intersection){
            //std::cout<<"Start BFS from "<<firstI<<" "<<firstJ<<std::endl;
            //std::cout<<"  Because "<<curI<<" "<<curJ<<" dir "<<d<<" "<<revDir(d)<<std::endl;
            std::vector<std::pair<int, int>> q = {{firstI, firstJ}};
            pixelColorStatus[firstI][firstJ] = PixelStatusEnum::colored;
            for(int front = 0; front < int(q.size()); front++){
                int fI = q[front].first, fJ = q[front].second;
                //std::cout<<"\tfront "<<fI<<" "<<fJ<<std::endl;
                for(int d = 0; d < (int) directions.size(); d++){
                    int nxtI = fI + directions[d].first;
                    int nxtJ = fJ + directions[d].second;
                    if(!insidePrimal(nxtI, nxtJ) || pixelColorStatus[nxtI][nxtJ] != PixelStatusEnum::intersection)
                        continue;
                    int dualI = fI + primalToDual[d].first;
                    int dualJ = fJ + primalToDual[d].second;
                    assert(insideDual(dualI, dualJ));
                    //std::cout<<"\t   Test "<<nxtI<<" "<<nxtJ<<" valid dual "<<dualI<<" "<<dualJ<<" "<<prevDir(d)<<std::endl;
                    if(validEdge[dualI][dualJ][prevDir(d)]){
                        //std::cout<<"\t   To "<<nxtI<<" "<<nxtJ<<" valid dual "<<dualI<<" "<<dualJ<<" "<<prevDir(d)<<std::endl;
                        pixelColorStatus[nxtI][nxtJ] = PixelStatusEnum::colored;
                        q.emplace_back(nxtI, nxtJ);
                    }
                }
            }
        }  
    }
}

std::vector<std::pair<int, int>> ImageTexture::dualBorder(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    std::vector<std::pair<int, int>> pixelsInBorder;
    {//upper edge
        int d = 0;
        int upperEdgeHeight = std::max(0, heightOffset);
        for(int j = 0; j < (int) inputImg.get_width() && j + widthOffset < this->imgWidth; j++){
            if(j + widthOffset < 0)
                continue;
            int a = upperEdgeHeight, b = j + widthOffset;   
            {
                int da = a + primalToDual[d].first;
                int db = b + primalToDual[d].second;
                pixelsInBorder.emplace_back(da, db);
            }
            {
                int da = a + primalToDual[prevDir(d)].first;
                int db = b + primalToDual[prevDir(d)].second;
                pixelsInBorder.emplace_back(da, db);
            }
        }
    }    
    {//left edge
        int d = 1;
        int leftEdgeWidth = std::max(0, widthOffset);
        for(int i = 0; i < (int) inputImg.get_height() && i + heightOffset < this->imgHeight; i++){
            if(i + heightOffset < 0)
                continue;
            int a = i + heightOffset, b = leftEdgeWidth;   
            {
                int da = a + primalToDual[d].first;
                int db = b + primalToDual[d].second;
                pixelsInBorder.emplace_back(da, db);
            }
            {
                int da = a + primalToDual[prevDir(d)].first;
                int db = b + primalToDual[prevDir(d)].second;
                pixelsInBorder.emplace_back(da, db);
            }
        }
    }
    {//lower edge
        int d = 2;
        int lowerEdgeHeight = std::min<int>(heightOffset + inputImg.get_height() - 1 , this->imgHeight - 1);
        for(int j = 0; j < (int) inputImg.get_width() && j + widthOffset < this->imgWidth; j++){
            if(j + widthOffset < 0)
                continue;
            int a = lowerEdgeHeight, b = j + widthOffset;   
            {
                int da = a + primalToDual[d].first;
                int db = b + primalToDual[d].second;
                pixelsInBorder.emplace_back(da, db);
            }
            {
                int da = a + primalToDual[prevDir(d)].first;
                int db = b + primalToDual[prevDir(d)].second;
                pixelsInBorder.emplace_back(da, db);
            }
        }
        //std::cout<<"lower edge is covered "<<lowerEdgeHeight<<std::endl;
    }
    {//right 
        int d = 3;
        int rightEdgeWidth = std::min<int>(widthOffset + inputImg.get_width() - 1, this->imgWidth - 1);
        for(int i = 0; i < (int) inputImg.get_height() && i + heightOffset < this->imgHeight; i++){
            if(i + heightOffset < 0)
                continue;
            int a = i + heightOffset, b = rightEdgeWidth;   
            {
                int da = a + primalToDual[d].first;
                int db = b + primalToDual[d].second;
                pixelsInBorder.emplace_back(da, db);
            }
            {
                int da = a + primalToDual[prevDir(d)].first;
                int db = b + primalToDual[prevDir(d)].second;
                pixelsInBorder.emplace_back(da, db);
            }
        }
    }
    sort(pixelsInBorder.begin(), pixelsInBorder.end());
    pixelsInBorder.resize(distance(pixelsInBorder.begin(), unique(pixelsInBorder.begin(), pixelsInBorder.end())));
    return pixelsInBorder;
}

std::vector<std::pair<int, int>> ImageTexture::findSCase2(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    std::vector<std::pair<int, int>> pixelsInS;
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
            if(a < 0 || b < 0)
                continue;
            if(pixelColorStatus[a][b] != PixelStatusEnum::intersection)
                continue;
            for(int d = 0; d < int(directions.size()); d++){
                int neiA = a + directions[d].first;
                int neiB = b + directions[d].second;
                if(insidePrimal(neiA, neiB) && pixelColorStatus[neiA][neiB] == PixelStatusEnum::newcolor){
                {
                    int da = a + primalToDual[d].first;
                    int db = b + primalToDual[d].second;
                    pixelsInS.emplace_back(da, db);
                }
                {
                    int da = a + primalToDual[prevDir(d)].first;
                    int db = b + primalToDual[prevDir(d)].second;
                    pixelsInS.emplace_back(da, db);
                }
                }
            }
        }
    if(pixelsInS.empty()){
        int leftEdgeWidth = std::max(0, widthOffset);
        int rightEdgeWidth = std::min<int>(widthOffset + inputImg.get_width() - 1, this->imgWidth - 1);
        int lowerEdgeHeight = std::min<int>(heightOffset + inputImg.get_height() - 1 , this->imgHeight - 1);
        int upperEdgeHeight = std::max(0, heightOffset);
        int h = (lowerEdgeHeight + upperEdgeHeight) / 2;
        int w = (rightEdgeWidth + leftEdgeWidth) / 2;
        pixelColorStatus[h][w] = PixelStatusEnum::newcolor;
        for(int d = 0; d < int(directions.size()); d++){
            int neiA = h + directions[d].first;
            int neiB = w + directions[d].second;
            if(insidePrimal(neiA, neiB)){
            {
                int da = h + primalToDual[d].first;
                int db = w + primalToDual[d].second;
                pixelsInS.emplace_back(da, db);
            }
            {
                int da = h + primalToDual[prevDir(d)].first;
                int db = w + primalToDual[prevDir(d)].second;
                pixelsInS.emplace_back(da, db);
            }
            }
        }

    }
    sort(pixelsInS.begin(), pixelsInS.end());
    pixelsInS.resize(distance(pixelsInS.begin(), unique(pixelsInS.begin(), pixelsInS.end())));
    return pixelsInS;
}