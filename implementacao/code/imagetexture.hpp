#pragma once
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
#include <unistd.h> //just for debug
#include <cstdlib> // just for debug
#include <iomanip> // just for debug
#include <string>

class ImageTexture{
public:
    ImageTexture(const png::image<png::rgb_pixel> & _img);
    ImageTexture(int width, int height);
    ImageTexture(int width, int height, uint64_t seed); // debug
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

    void copyPixelsNewColor(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg, bool case2 = false);
    bool inImgBorder(int i, int j, int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);    
    bool insidePrimal(int i, int j);  
    bool insideDual(int i, int j);
    bool insideImg(int i, int j, const png::image<png::rgb_pixel> &img);
    class Intersection{
        public:
            // counter clockwise, starting with upper neighbor
            std::vector<std::pair<int, int>> interPixels;
            Intersection(const std::vector<std::pair<int,int>> &pixels = {});
    };
    //Case 1 auxiliar variables
    std::vector<std::vector<bool>> inSubgraph;
    std::vector<std::vector<std::array<long double, 4>>> edgesCosts;    
    std::vector<std::vector<long double>> dist;
    std::vector<std::vector<bool>> vis;
    std::vector<std::vector<int>> parent;
    std::vector<std::vector<std::array<bool, 4>>> validEdge;
    std::vector<std::vector<bool>> isT;
    std::vector<std::vector<bool>> isS;

    //int heightOffset, int widthOffset for debug, will change later
    std::pair<std::pair<int, int>, std::pair<int, int> > findSTInIntersectionCase1(Intersection &inter, int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    std::vector<Intersection> findIntersections(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    void markMinABCut(std::pair<int, int> S, std::pair<int, int> T, const ImageTexture::Intersection &inter, int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    //MarkMinABCut auxiliar functions
    std::vector<std::pair<int, int>> markIntersectionCellsInDual(const ImageTexture::Intersection &inter);
    void markIntersectionEdgeCostsInDual(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg, const std::vector<std::pair<int, int>> &inDual);
    std::vector<std::pair<int,int>> findSTPath(const std::vector<std::pair<int,int>> &S, const std::vector<std::pair<int,int>> &T);
    void markLeftOfMinCut(const std::vector<std::pair<int, int>> &cut);
    
    //Case 2 auxiliar variables
    std::vector<std::vector<bool>> inS;
    std::vector<std::vector<int>> inStPath;
    constexpr static std::array<int, 4> edgesToOriginalGraph = {edgeType::originalGraph,edgeType::originalGraph,edgeType::originalGraph,edgeType::originalGraph};
    std::array<std::vector<std::vector<std::array<int, 4>>>,2> edgeTo;
    std::array<std::vector<std::vector<long double>>, 2> distCase2;
    std::array<std::vector<std::vector<int>>, 2> visCase2;
    std::array<std::vector<std::vector<int>>, 2> seenCase2;
    std::array<std::vector<std::vector<int>>, 2> parentCase2;

    //Case 2 auxiliar functions
    std::vector<std::pair<int, int>> dualBorder(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    std::vector<std::pair<int, int>> findSCase2(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    std::pair<long double, std::vector<std::array<int,3>>> minCutCycle(int left, int right, const std::vector<std::pair<int, int>> &stPath, int &visited);    
    std::pair<long double, std::vector<std::array<int,3>>> findMinFCycle(const std::pair<int,int> &F, int visited);
};
