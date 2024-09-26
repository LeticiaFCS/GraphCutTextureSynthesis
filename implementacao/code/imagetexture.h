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
    const uint64_t rngSeed;
    std::mt19937_64 rng;
    enum PixelStatusEnum{
        colored, intersection, newcolor, notcolored
    };
    enum edgeType{
        originalGraph, copyGraph, invalid
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
    void markIntersectionEdgeCostsInDual(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg, const std::vector<std::pair<int, int>> &inDual, 
         const std::vector<std::vector<bool>> &inSubgraph, std::vector<std::vector<std::array<long double, 4>>> &edgesCosts);
    std::vector<std::pair<int,int>> findSTPath(const std::vector<std::pair<int,int>> &S, const std::vector<std::pair<int,int>> &T, 
         const std::vector<std::vector<bool>> &inSubgraph, const std::vector<std::vector<std::array<long double, 4>>> &edgesCosts, 
         std::vector<std::vector<long double>> &dist, std::vector<std::vector<bool>> &vis, std::vector<std::vector<int>> &parent);
    void markLeftOfMinCut(std::vector<std::pair<int, int>> &cut, std::vector<std::vector<int>> &parent, const std::vector<std::vector<std::array<bool, 4>>> &validEdge);
    //Case 2 auxiliar functions
    std::vector<std::pair<int, int>> dualBorder(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    std::vector<std::pair<int, int>> findSCase2(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    //std::vector<std::pair<int, int>> findSCase2(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    std::pair<long double, std::vector<std::array<int,3>>> minCutCycle(const std::vector<std::vector<bool>> &inSubgraph, const std::vector<std::vector<std::array<long double, 4>>> &edgesCosts,
         std::array<std::vector<std::vector<std::array<int, 4>>>,2> &edgeTo, int left, int right, const std::vector<std::pair<int, int>> &stPath,
         std::array<std::vector<std::vector<long double>>, 2> &dist, std::array<std::vector<std::vector<int>>, 2> &vis, 
         std::array<std::vector<std::vector<int>>, 2> &seen, std::array<std::vector<std::vector<int>>, 2> &parent, 
         const std::vector<std::vector<int>> &inStPath, int &visited);    
    std::pair<long double, std::vector<std::array<int,3>>> findMinFCycle(const std::pair<int,int> &F, const std::vector<std::vector<bool>> &inSubgraph, const std::vector<std::vector<std::array<long double, 4>>> &edgesCosts, 
         const std::array<std::vector<std::vector<std::array<int, 4>>>,2> &edgeTo, std::array<std::vector<std::vector<long double>>, 2> &dist, std::array<std::vector<std::vector<int>>, 2> &vis,
         std::array<std::vector<std::vector<int>>, 2> &seen, std::array<std::vector<std::vector<int>>, 2> &parent, int visited);
};

/*
Constructors
*/
ImageTexture::ImageTexture(const png::image<png::rgb_pixel> & _img) 
    : 
    rngSeed(std::chrono::steady_clock::now().time_since_epoch().count()),
    rng(rngSeed),
    outputImg(_img), 
    imgWidth(_img.get_width()),
    imgHeight(_img.get_height()),
    pixelColorStatus(_img.get_width(), std::vector<PixelStatusEnum>(_img.get_height(), PixelStatusEnum::notcolored)){
    std::cout<<"Seed is "<<rngSeed<<std::endl;
}
ImageTexture::ImageTexture(int width, int height) 
    :  
    //rngSeed(7847891524704), //https://www.geogebra.org/calculator/xafaqxnx
    //rngSeed(10698447507461),  //https://www.geogebra.org/calculator/fhtqafpt
    //rngSeed(49102102093506),  // https://www.geogebra.org/calculator/kesrjeex //too small
    //rngSeed(23796671245129),  //path empty
    //rngSeed(31740571079601),  //path empty - solved
    //rngSeed(39937168697097),
    rngSeed(31808842946432),
    //rngSeed(std::chrono::steady_clock::now().time_since_epoch().count()),
    rng(rngSeed),
    outputImg(width, height), 
    imgWidth(width),
    imgHeight(height), 
    pixelColorStatus(width, std::vector<PixelStatusEnum>(height, PixelStatusEnum::notcolored)){
    std::cout<<"Seed is "<<rngSeed<<std::endl;    
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
    static std::uniform_int_distribution<int> nextHeight(-(int) inputImg.get_height() + 1, imgHeight-1);
    static std::uniform_int_distribution<int> nextWidth(-(int) inputImg.get_width() + 1, imgWidth-1);
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
    static std::vector<std::vector<bool>> inS(imgHeight + 1, std::vector<bool>(imgWidth + 1));
    static std::vector<std::vector<int>> inStPath(imgHeight + 1, std::vector<int>(imgWidth + 1, -1));
    static std::vector<std::vector<std::array<long double, 4>>> edgesCosts(imgHeight + 1, std::vector<std::array<long double, 4>>(imgWidth + 1));    
    static std::vector<std::vector<long double>> dist(imgHeight + 1, std::vector<long double>(imgWidth + 1));
    static std::vector<std::vector<bool>> vis(imgHeight + 1, std::vector<bool>(imgWidth + 1));
    static std::vector<std::vector<int>> parent(imgHeight + 1, std::vector<int>(imgWidth + 1, -1));
    // enum edgeType{
    //     originalGraph, copyGraph, invalid
    // };
    const static std::array<int, 4> edgesToOriginalGraph = {edgeType::originalGraph,edgeType::originalGraph,edgeType::originalGraph,edgeType::originalGraph};
    static std::array<std::vector<std::vector<std::array<int, 4>>>,2> edgeTo(
        {std::vector<std::vector<std::array<int, 4>>>{imgHeight + 1, std::vector<std::array<int, 4>>(imgWidth + 1, edgesToOriginalGraph)},
         std::vector<std::vector<std::array<int, 4>>>{imgHeight + 1, std::vector<std::array<int, 4>>(imgWidth + 1, edgesToOriginalGraph)}}
        );
   
    Intersection intersection;
    /*find intersection*/
    auto intersections = findIntersections(heightOffset, widthOffset, inputImg);
    assert(intersections.size() == 1);
    intersection = intersections[0];
    
    /*mark cells in dual of intersection and mark edges costs*/
    std::vector<std::pair<int, int>> cellsInDual = markIntersectionCellsInDual(intersection, inSubgraph);
    markIntersectionEdgeCostsInDual(heightOffset, widthOffset, inputImg, cellsInDual, inSubgraph, edgesCosts);

    /*find ST path*/
    auto S = findSCase2(heightOffset, widthOffset, inputImg);
    if(S.empty()){ // degenerate case
        for(auto [x, y] : intersection.interPixels)
            pixelColorStatus[x][y] = PixelStatusEnum::colored;
        std::cout<<"degenerate case";
        return;
    }
    auto T = dualBorder(heightOffset, widthOffset, inputImg);
    auto tsPath = findSTPath(S, T, inSubgraph, edgesCosts, dist, vis, parent);
    

    // exit by right - original graph
    // enter by left - copy graph
    /*mark ST Path*/{
        assert((int(tsPath.size()) >= 2, "tsPath lenght should be at least 2"));
        std::cout<<"Ts path: "<<std::endl;
        for(auto [x, y] : tsPath){
            std::cout<<" ("<<x<<", "<<y<<"),"<<std::endl;
        }
        std::cout<<std::endl;
        for(auto [x, y] : S)
            inS[x][y] = true;
        for(int i = 0; i < (int) tsPath.size(); i++){
            auto [x, y] = tsPath[i];
            inStPath[x][y] = i;
        }

        auto [secondX, secondY] = tsPath[tsPath.size() - 2];
        int firstParent = (parent[secondX][secondY]);
        std::cout<<"segundoooo "<<secondX<<" "<<secondY<<" "<<parent[secondX][secondY]<<std::endl;
        for(int i =0; i < int(tsPath.size()) - 1; i++){
            auto [x, y] = tsPath[i];
            auto [nextX, nextY] = tsPath[i+1];
            int d = parent[x][y];
            edgeTo[edgeType::copyGraph][nextX][nextY][revDir(d)] = edgeType::copyGraph;
            edgeTo[edgeType::copyGraph][x][y][d] = edgeType::copyGraph;
            std::cout<<"creating copy path"<<x<<" "<<y<<" "<<d<<" --> "<<nextX<<" "<<nextY<<" "<<revDir(d)<<std::endl;
        }
        {// dealing with edges of T
            int d = prevDir( revDir(parent[secondX][secondY]) );
            auto [lastX, lastY] = tsPath.back();
            for(int i = 0; i < 3; i++){
                int nextX = lastX + directions[d].first, nextY = lastY + directions[d].second;
                std::cout<<"LAST "<<lastX<<" "<<lastY<<" --> "<<nextX<<" "<<nextY<<std::endl;
                if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
                    std::cout<<"INVALID LAST "<<"("<<nextX<<", "<<nextY<<"), ("<<lastX<<", "<<lastY<<"),"<<std::endl;
                    edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::invalid;
                    if(inS[nextX][nextY]) break;
                }
                d = prevDir(d);
            }
            d = nextDir( revDir(parent[secondX][secondY]) );
            for(int i = 0; i < 3; i++){
                int nextX = lastX + directions[d].first, nextY = lastY + directions[d].second;
                std::cout<<"LAST "<<lastX<<" "<<lastY<<" --> "<<nextX<<" "<<nextY<<std::endl;
                if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
                    std::cout<<"COPY LAST "<<"("<<nextX<<", "<<nextY<<"), ("<<lastX<<", "<<lastY<<"),"<<std::endl;
                    edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::copyGraph;
                    edgeTo[edgeType::originalGraph][lastX][lastY][d] = edgeType::invalid;
                    if(inS[nextX][nextY]) break;
                }
                d = nextDir(d);
            }

        }
        // edges from original graph to copy graph, to close the cycle
        for(int i =0; i < int(tsPath.size()) - 1; i++){
            auto [x, y] = tsPath[i];
            int d = prevDir(parent[x][y]);
            int nextX = x + directions[d].first, nextY = y + directions[d].second;
            if((i > 0 && tsPath[i-1] == std::make_pair(nextX, nextY)) || (i == 0 && d == revDir(parent[x][y])))
                continue;
            if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
                std::cout<<"Copy ("<<nextX<<", "<<nextY<<") , ("<<x<<", "<<y<<"),"<<std::endl;
                edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::copyGraph;
                edgeTo[edgeType::originalGraph][x][y][d] = edgeType::invalid;
            }
            d = prevDir(d);
            nextX = x + directions[d].first, nextY = y + directions[d].second;
            if((i > 0 && tsPath[i-1] == std::make_pair(nextX, nextY))|| (i == 0 && d == revDir(parent[x][y])))
                continue;
            if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
                std::cout<<"Copy ("<<nextX<<", "<<nextY<<") , ("<<x<<", "<<y<<"),"<<std::endl;
                edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::copyGraph;
                edgeTo[edgeType::originalGraph][x][y][d] = edgeType::invalid;
            }
        }
        // invalid edges, so we must go to the left first
        for(int i =0; i < int(tsPath.size()) - 1; i++){
            auto [x, y] = tsPath[i];
            std::cout<<"X "<<x<<" Y "<<y<<std::endl;
            std::cout<<"par "<<parent[x][y]<<std::endl;
            int d = nextDir(parent[x][y]);
            int nextX = x + directions[d].first, nextY = y + directions[d].second;
            if((i > 0 && tsPath[i-1] == std::make_pair(nextX, nextY)) || (i == 0 && d == revDir(parent[x][y])))
                continue;
            if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
                std::cout<<"Invalid ("<<nextX<<", "<<nextY<<") , ("<<x<<", "<<y<<"),"<<std::endl;
                edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::invalid;
            }
            d = nextDir(d);
            nextX = x + directions[d].first, nextY = y + directions[d].second;
            std::cout<<"d "<<d<<" next "<<nextX<<" "<<nextY<<std::endl;
            if((i > 0 && tsPath[i-1] == std::make_pair(nextX, nextY)) || (i == 0 && d == revDir(parent[x][y])))
                continue;
            if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
                std::cout<<"Invalid ("<<nextX<<", "<<nextY<<") , ("<<x<<", "<<y<<"),"<<std::endl;
                edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::invalid;
            }
        }
    }

    std::pair<long double, std::vector<std::array<int,3>>> minCut;
    { //min cut 
        static std::array<std::vector<std::vector<long double>>, 2> dist({
            std::vector<std::vector<long double>>(imgHeight + 1, std::vector<long double>(imgWidth + 1)),
            std::vector<std::vector<long double>>(imgHeight + 1, std::vector<long double>(imgWidth + 1))
        });
        static std::array<std::vector<std::vector<int>>, 2> vis({
            std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1)),
            std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1))
        });
        static std::array<std::vector<std::vector<int>>, 2> seen({
            std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1)),
            std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1))
        });    
        static std::array<std::vector<std::vector<int>>, 2> parent({
            std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1, -1)),
            std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1, -1))
        } );
        int visited = 1;   
        minCut = minCutCycle(inSubgraph, edgesCosts, edgeTo, 0, tsPath.size(), tsPath, dist, vis, seen, parent, inStPath, visited);
        std::cout<<"MIN CUT CYCLE: "<<minCut.first<<std::endl;
        for(auto [g,i,j] : minCut.second){
            std::cout<<g<<" "<<i<<" "<<j<<std::endl;
        }

        std::cout<<std::endl;
        std::cout<<"LIMPANDO: "<<std::endl;
        std::cout<<" -- "<<heightOffset<<" --> "<<heightOffset + inputImg.get_height()<<std::endl;
        std::cout<<" -- "<<widthOffset<<" --> "<<widthOffset + inputImg.get_width()<<std::endl;
        //std::cout<<" lim "<<imgHeight<<" "<<imgWidth<<std::endl;
        for(int i = heightOffset; i <= heightOffset + (int)inputImg.get_height(); i++){
            //std::cout<<"i is "<<i<<std::endl;
            for(int j = widthOffset; j <= widthOffset + (int)inputImg.get_width(); j++){
                //std::cout<<"j is "<<j<<std::endl;
                //std::cout<<"Tentando Limpar "<<i<<" "<<j<<" ---> "<<insideDual(i, j)<<std::endl;
                if(insideDual(i, j)){
                    //std::cout<<"Limpa "<<i<<" "<<j<<std::endl;
                    dist[0][i][j] = 0;
                    dist[1][i][j] = 0;
                    vis[0][i][j] = 0;
                    vis[1][i][j] = 0;
                    seen[0][i][j] = 0;
                    seen[1][i][j] = 0;
                    parent[0][i][j] = -1;
                    parent[1][i][j] = -1;
                }
            }
        }

    }

    /*unmark ST Path*/{

        auto [secondX, secondY] = tsPath[tsPath.size() - 2];
        for(int i =0; i < int(tsPath.size()) - 1; i++){
            auto [x, y] = tsPath[i];
            auto [nextX, nextY] = tsPath[i+1];
            int d = parent[x][y];
            edgeTo[edgeType::copyGraph][nextX][nextY][revDir(d)] = edgeType::originalGraph;
            edgeTo[edgeType::copyGraph][x][y][d] = edgeType::originalGraph;
        }
        {// dealing with edges of T
            int d = prevDir( revDir(parent[secondX][secondY]) );
            auto [lastX, lastY] = tsPath.back();
            for(int i = 0; i < 3; i++){
                int nextX = lastX + directions[d].first, nextY = lastY + directions[d].second;
                if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
                    edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::originalGraph;
                    if(inS[nextX][nextY]) break;
                }
                d = prevDir(d);
            }
            d = nextDir( revDir(parent[secondX][secondY]) );
            for(int i = 0; i < 3; i++){
                int nextX = lastX + directions[d].first, nextY = lastY + directions[d].second;
                if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
                    edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::originalGraph;
                    edgeTo[edgeType::originalGraph][lastX][lastY][d] = edgeType::originalGraph;
                    if(inS[nextX][nextY]) break;
                }
                d = nextDir(d);
            }

        }
        // edges from original graph to copy graph, to close the cycle
        for(int i =0; i < int(tsPath.size()) - 1; i++){
            auto [x, y] = tsPath[i];
            int d = prevDir(parent[x][y]);
            int nextX = x + directions[d].first, nextY = y + directions[d].second;
            if((i > 0 && tsPath[i-1] == std::make_pair(nextX, nextY)) || (i == 0 && d == revDir(parent[x][y])))
                continue;
            if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
                edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::originalGraph;
                edgeTo[edgeType::originalGraph][x][y][d] = edgeType::originalGraph;
            }
            d = prevDir(d);
            nextX = x + directions[d].first, nextY = y + directions[d].second;
            if((i > 0 && tsPath[i-1] == std::make_pair(nextX, nextY))|| (i == 0 && d == revDir(parent[x][y])))
                continue;
            if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
                edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::originalGraph;
                edgeTo[edgeType::originalGraph][x][y][d] = edgeType::originalGraph;
            }
        }
        // invalid edges, so we must go to the left first
        for(int i =0; i < int(tsPath.size()) - 1; i++){
            auto [x, y] = tsPath[i];
            int d = nextDir(parent[x][y]);
            int nextX = x + directions[d].first, nextY = y + directions[d].second;
            if((i > 0 && tsPath[i-1] == std::make_pair(nextX, nextY)) || (i == 0 && d == revDir(parent[x][y])))
                continue;
            if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
                edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::originalGraph;
            }
            d = nextDir(d);
            nextX = x + directions[d].first, nextY = y + directions[d].second;
            if((i > 0 && tsPath[i-1] == std::make_pair(nextX, nextY)) || (i == 0 && d == revDir(parent[x][y])))
                continue;
            if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
                edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::originalGraph;
            }
        }
        for(auto [x, y] : tsPath)
            inStPath[x][y] = -1;
        for(auto [x, y] : S)
            inS[x][y] = false;
    }

    /*clean dijkstra matrices*/{
        for(auto [i,j] : intersection.interPixels)
            for(auto [di, dj] : primalToDual)
                if(insideDual(i + di, j + dj)){
                    parent[i + di][j + dj] = -1;
                    vis[i + di][j + dj] = false;
                }
    }

    // /*mark ST path*/{
    //     //std::cout<<"ST path"<<std::endl;
    //     int lastD = -1;
    //     for(auto [curI, curJ] : tsPath){
    //         if(lastD >= 0){
    //             validEdge[curI][curJ][lastD] = false;
    //         }
    //         int d = parent[curI][curJ];
    //         if(d >= 0){
    //             //std::cout<<std::fixed<<std::setprecision(10)<<"\t"<<curI<<" "<<curJ<<" -- dir "<<parent[curI][curJ]<<" distance "<<dist[curI][curJ]<<std::endl;
    //             validEdge[curI][curJ][d] = false;
    //             lastD = revDir(d);
    //         }
    //     }
    // }    
    
    /*unmark ST Path*/{
        for(auto [x, y] : tsPath)
            inStPath[x][y] = -1;
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

    /*unmark cells in dual of intersection*/
    // { //unmark ST path
    //     int lastD = -1;
    //     for(auto [curI, curJ] : tsPath){
    //         if(lastD >= 0){
    //             validEdge[curI][curJ][lastD] = true;
    //         }
    //         int d = parent[curI][curJ];
    //         if(d >= 0){
    //             validEdge[curI][curJ][d] = true;
    //             lastD = revDir(d);
    //         }
    //     }
    // }
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
    //exit(0);

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
    //render("../output/output.png"); //debug descomentar!!!
    //usleep(800000);
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
    return i == heightOffset || i == std::min<int>(imgHeight - 1, heightOffset + (int) inputImg.get_height() - 1) 
        || j == widthOffset  || j == std::min<int>(imgWidth - 1, widthOffset + (int) inputImg.get_width() - 1);
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
        //usleep(800000);
        //std::cout<<"OFFSET height "<<heightOffset<<" width "<<widthOffset<<std::endl;
        for(auto [x,y] : inter.interPixels){
            outputImg[x][y] = png::rgb_pixel(0,155,0);
        }
        //render("../output/output.png"); //debug descomentar!!
        //usleep(2000000);

    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
            if(a < 0 || b < 0)
                continue;
            if(pixelColorStatus[a][b] == PixelStatusEnum::newcolor)
                outputImg[a][b] = png::rgb_pixel(0,0,155);
            
        }
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
        std::cout<<" T ("<<h<<", "<<w<<"),"<<std::endl;
        isT[h][w] = true;
    }
    using qtype = std::tuple<long double, int, int>;
    std::priority_queue<qtype, std::vector<qtype>, std::greater<qtype>> Q;
    for(auto [h, w] : S){
        std::cout<<" S ("<<h<<", "<<w<<"),"<<std::endl;
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
            //std::cout<<" --> "<<nextI<<" "<<nextJ<<std::endl;
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
        int lowerEdgeHeight = std::min<int>(heightOffset + (int) inputImg.get_height() - 1 , this->imgHeight - 1);
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
        int rightEdgeWidth = std::min<int>(widthOffset + (int) inputImg.get_width() - 1, this->imgWidth - 1);
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
        int rightEdgeWidth = std::min<int>(widthOffset + (int) inputImg.get_width() - 1, this->imgWidth - 1);
        int lowerEdgeHeight = std::min<int>(heightOffset + (int) inputImg.get_height() - 1 , this->imgHeight - 1);
        int upperEdgeHeight = std::max(0, heightOffset);
        if(lowerEdgeHeight >= upperEdgeHeight) // degenerate case
            return {};
        if(rightEdgeWidth >= leftEdgeWidth) // degenerate case
            return {};
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

std::pair<long double, std::vector<std::array<int,3>>> ImageTexture::findMinFCycle(const std::pair<int,int> &F, const std::vector<std::vector<bool>> &inSubgraph, const std::vector<std::vector<std::array<long double, 4>>> &edgesCosts, const std::array<std::vector<std::vector<std::array<int, 4>>>,2> &edgeTo, std::array<std::vector<std::vector<long double>>, 2> &dist, std::array<std::vector<std::vector<int>>, 2> &vis, std::array<std::vector<std::vector<int>>, 2> &seen, std::array<std::vector<std::vector<int>>, 2> &parent, int visited){
    std::array<int, 3> S = {0, F.first, F.second};
    std::array<int, 3> T = {1, F.first, F.second};
    using qtype = std::tuple<long double, int, int, int>;
    std::priority_queue<qtype, std::vector<qtype>, std::greater<qtype>> Q;
    parent[S[0]][S[1]][S[2]] = -2;
    dist[S[0]][S[1]][S[2]] = 0;
    Q.emplace(0, S[0], S[1], S[2]);
    std::cout<<"START DIJKSTRA MIN F CYCLE"<<std::endl;
    std::cout<<" face "<<S[1]<<" "<<S[2]<<std::endl;
    std::vector<std::array<int,3>> path;
    while(!Q.empty()){
        long double pathCost;
        int g, i, j;
        std::tie(pathCost, g, i, j) = Q.top();
        Q.pop();
        //std::cout<<"Test "<<g<<" "<<i<<" "<<j<<std::endl;
        assert(i < vis[0].size());
        assert(j < vis[0][i].size());
        //std::cout<<" is vis "<<vis[g][i][j]<<std::endl;
        if(vis[g][i][j] == visited)
            continue;
        //std::cout<<"Dijkstra "<<g<<" "<<i<<" "<<j<<std::endl;
        vis[g][i][j] = visited;
        if(T == std::array{g,i,j}){
            path = {{g,i,j}};
            break;
        }
        for(int d = 0; d < int(directions.size()); d++){
            int nextI = i + directions[d].first, nextJ = j + directions[d].second;
            //std::cout<<"to --> "<<nextI<<" "<<nextJ<<std::endl;
            int nextG = edgeTo[g][i][j][d];
            if(nextG == 2)
                continue;
            if(insideDual(nextI, nextJ) && inSubgraph[nextI][nextJ] && vis[nextG][nextI][nextJ] != visited){
                long double curCost = edgesCosts[i][j][d];
                // if(isS[i][j] && isT[nextI][nextJ]) //avoid direct empty cut
                //     continue;
                if(seen[nextG][nextI][nextJ] != visited || pathCost + curCost < dist[nextG][nextI][nextJ]){
                    seen[nextG][nextI][nextJ] = visited;
                    parent[nextG][nextI][nextJ] = revDir(d)*10+g;
                    dist[nextG][nextI][nextJ] = pathCost + curCost; // avoid overflow
                    Q.emplace(dist[nextG][nextI][nextJ], nextG, nextI, nextJ);
                }
            }
        }
    }
    assert(("path is empty!", !path.empty()));
    std::cout<<" path = ";
    std::cout<<dist[T[0]][T[1]][T[2]]<<std::endl;
    // // only path is direct path between S and T
    // if(!path.empty()){
    //     //std::cout<<"PATH IS EMPTY"<<std::endl;
    //     bool ok = false;
    //     for(auto [i, j] : S){
    //         for(int d = 0; d < int(directions.size()); d++){
    //             int nextI = i + directions[d].first, nextJ = j + directions[d].second;
    //             if(insideDual(nextI, nextJ) && inSubgraph[nextI][nextJ] && !vis[nextI][nextJ] && isT[nextI][nextJ]){
    //                 parent[nextI][nextJ] = revDir(d);
    //                 path = {{nextI, nextJ}};
    //                 break;
    //             }
    //         }
    //         if(!path.empty())
    //             break;
    //     }
    // }
    int curG = T[0], curI, curJ;
    std::tie(curG, curI, curJ) = std::tuple_cat(path[0]);
    while(parent[curG][curI][curJ] != -2){
        int d = parent[curG][curI][curJ]/10;
        //std::cout<<std::fixed<<std::setprecision(10)<<"\t"<<curG<<" "<<curI<<" "<<curJ<<" -- dir "<<d<<" distance "<<dist[curG][curI][curJ]<<std::endl;
        curG = parent[curG][curI][curJ]%10;
        //std::cout<<"curG = "<<curG<<std::endl;
        assert(("direction must be valid!", 0 <= d && d < int(directions.size())));
        std::tie(curI, curJ) = std::make_tuple(curI + directions[d].first, curJ + directions[d].second);
        //std::cout<<" going to "<<curG<<" "<<curI<<" "<<curJ<<std::endl;
        path.push_back({curG, curI, curJ});
    }

    return {dist[T[0]][T[1]][T[2]], path};
}


std::pair<long double, std::vector<std::array<int,3>>> ImageTexture::minCutCycle(const std::vector<std::vector<bool>> &inSubgraph, const std::vector<std::vector<std::array<long double, 4>>> &edgesCosts, std::array<std::vector<std::vector<std::array<int, 4>>>,2> &edgeTo, int left, int right, const std::vector<std::pair<int, int>> &stPath, std::array<std::vector<std::vector<long double>>, 2> &dist, std::array<std::vector<std::vector<int>>, 2> &vis, std::array<std::vector<std::vector<int>>, 2> &seen, std::array<std::vector<std::vector<int>>, 2> &parent, const std::vector<std::vector<int>> &inStPath, int &visited){
    visited++;
    static std::vector<std::vector<int>> inCutCycle(imgHeight + 1, std::vector<int>(imgWidth + 1));
    
    std::cout<<"min cut cycle dac "<<left<<" "<<right<<std::endl;
    std::cout<<"visited = "<<visited<<std::endl;

    int f_mid = (left + right) / 2;
    std::cout<<"f_mid "<<f_mid<<std::endl;
    
    auto curCutCycle = findMinFCycle(stPath[f_mid], inSubgraph, edgesCosts, edgeTo, dist, vis, seen, parent, visited);
    std::cout<<"cur cycle dac: \n";
    for(auto [g, i, j] : curCutCycle.second){
        std::cout<<g<<" ("<<i<<", "<<j<<"),"<<std::endl;
    }
    std::cout<<std::endl;

    auto answerCut = curCutCycle;

    std::vector<int> parentsOfCutCycle(curCutCycle.second.size());
    for(int i = 0; i < (int) curCutCycle.second.size(); i++){
        auto [g, x, y] = curCutCycle.second[i];
        parentsOfCutCycle[i] = parent[g][x][y];
        inCutCycle[x][y]++;
    }  
    
    if(left < f_mid){
        std::cout<<"marking invalid left"<<std::endl;
        std::vector<std::array<int, 5>> oldEdgeValues;
        for(int i =0; i < int(curCutCycle.second.size()) - 1; i++){
            auto [g, x, y] = curCutCycle.second[i];
            assert(parentsOfCutCycle[i] >= 0);
            int d = nextDir(parentsOfCutCycle[i]);
            int nextX = x + directions[d].first, nextY = y + directions[d].second;
            if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY] && 
              (inStPath[nextX][nextY] == -1 || inStPath[nextX][nextY] >= f_mid)){
                if(inCutCycle[nextX][nextY])
                        continue;
                std::cout<<"Invalid ("<<x<<", "<<y<<") , ("<<nextX<<", "<<nextY<<") --> g is "<<g<<std::endl;
                oldEdgeValues.push_back({edgeType::originalGraph,x,y,d,edgeTo[edgeType::originalGraph][x][y][d]});
                edgeTo[edgeType::originalGraph][x][y][d] = edgeType::invalid;
                oldEdgeValues.push_back({edgeType::copyGraph,x,y,d,edgeTo[edgeType::copyGraph][x][y][d]});
                edgeTo[edgeType::copyGraph][x][y][d] = edgeType::invalid;
            }
            d = nextDir(d);
            nextX = x + directions[d].first, nextY = y + directions[d].second;
            if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY] && 
              (inStPath[nextX][nextY] == -1 || inStPath[nextX][nextY] >= f_mid)){
                if(inCutCycle[nextX][nextY])
                        continue;
                std::cout<<"Invalid ("<<x<<", "<<y<<") , ("<<nextX<<", "<<nextY<<") --> g is "<<g<<std::endl;
                oldEdgeValues.push_back({edgeType::originalGraph,x,y,d,edgeTo[edgeType::originalGraph][x][y][d]});
                edgeTo[edgeType::originalGraph][x][y][d] = edgeType::invalid;
                oldEdgeValues.push_back({edgeType::copyGraph,x,y,d,edgeTo[edgeType::copyGraph][x][y][d]});
                edgeTo[edgeType::copyGraph][x][y][d] = edgeType::invalid;
            }
        }
        auto leftCutCycle = minCutCycle(inSubgraph,edgesCosts,edgeTo, left, f_mid, stPath, dist, vis, seen, parent, inStPath, visited);
        for(auto [g,x,y,dir,value] : oldEdgeValues){
            edgeTo[g][x][y][dir] = value;
        }
        answerCut = min(answerCut, leftCutCycle);
    }
    //right
    if(f_mid + 1 < right){
        std::cout<<"marking invalid right"<<std::endl;
        std::vector<std::array<int, 5>> oldEdgeValues;
        for(int i =0; i < int(curCutCycle.second.size()) - 1; i++){
            auto [g, x, y] = curCutCycle.second[i];
            assert(parentsOfCutCycle[i] >= 0);
            int d = prevDir(parentsOfCutCycle[i]);
            int nextX = x + directions[d].first, nextY = y + directions[d].second;
            if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY] && 
              (inStPath[nextX][nextY] == -1 || inStPath[nextX][nextY] <= f_mid)){
                if(inCutCycle[nextX][nextY])
                        continue;
                std::cout<<"Invalid ("<<x<<", "<<y<<") , ("<<nextX<<", "<<nextY<<") --> g is "<<g<<std::endl;
                oldEdgeValues.push_back({edgeType::originalGraph,x,y,d,edgeTo[edgeType::originalGraph][x][y][d]});
                edgeTo[edgeType::originalGraph][x][y][d] = edgeType::invalid;
                oldEdgeValues.push_back({edgeType::copyGraph,x,y,d,edgeTo[edgeType::copyGraph][x][y][d]});
                edgeTo[edgeType::copyGraph][x][y][d] = edgeType::invalid;
            }
            d = prevDir(d);
            nextX = x + directions[d].first, nextY = y + directions[d].second;
            if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY] && 
              (inStPath[nextX][nextY] == -1 || inStPath[nextX][nextY] <= f_mid)){
                if(inCutCycle[nextX][nextY])
                        continue;
                std::cout<<"Invalid ("<<x<<", "<<y<<") , ("<<nextX<<", "<<nextY<<") --> g is "<<g<<std::endl;
                oldEdgeValues.push_back({edgeType::originalGraph,x,y,d,edgeTo[edgeType::originalGraph][x][y][d]});
                edgeTo[edgeType::originalGraph][x][y][d] = edgeType::invalid;
                oldEdgeValues.push_back({edgeType::copyGraph,x,y,d,edgeTo[edgeType::copyGraph][x][y][d]});
                edgeTo[edgeType::copyGraph][x][y][d] = edgeType::invalid;
            }
        }
        auto rightCutCycle = minCutCycle(inSubgraph,edgesCosts,edgeTo, f_mid + 1, right, stPath, dist, vis, seen, parent, inStPath, visited);
        for(auto [g,x,y,dir,value] : oldEdgeValues){
            edgeTo[g][x][y][dir] = value;
        }
        answerCut = min(answerCut, rightCutCycle);
    }
    for(auto [g, i, j] : curCutCycle.second){
        inCutCycle[i][j]--;
    }
    return answerCut;
}