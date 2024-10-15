/**
 * @file imagetexture.hpp
 * @author Letícia Freire Carvalho de Sousa
 * @brief Header file
 * @version 0.1
 * @date 2024-10-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */
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

/**
 * @brief 
 * 
 */
class ImageTexture{
public:
    /**
     * @brief Construct a new Image Texture object
     * 
     * Time Complexity: O(width &times; height)
     * 
     * @param _img png::image object where the texture will be constructed
     */
    ImageTexture(const png::image<png::rgb_pixel> & _img);

    /**
     * @brief Construct a new Image Texture object
     * 
     * Time Complexity: O(width &times; height)
     * 
     * @param width width of the texture that will be conctructed (in pixels)
     * @param height height of the texture that will be conctructed (in pixels)
     */
    ImageTexture(int width, int height);

    /**
     * @brief An iteration of patch fitting
     * 
     * Time Complexity: O(width &times; height &times; log<sup>2</sup>(width &times; height ))
     * 
     * @param inputImg png::image object from which the patch will be copied
     */
    void patchFittingIteration(const png::image<png::rgb_pixel> &inputImg);

    /**
     * @brief An iteration of patch fitting
     * 
     * Time Complexity: O(width &times; height &times; log<sup>2</sup>(width &times; height ))
     * 
     * @param file_name file name of the png image from which the patch will be copied
     */
    void patchFittingIteration(const std::string &file_name);

    /**
     * @brief Runs 'CntIterations' iterations of the patch fitting
     * 
     * Time Complexity: O(CntIterations &times; (width &times; height &times; log<sup>2</sup>(width &times; height )))
     * 
     * @param inputImg png::image object from which the patch will be copied
     * @param CntIterations number of iterations
     */
    void patchFitting(const png::image<png::rgb_pixel> &inputImg, int CntIterations = 100000);

    /**
     * @brief Runs 'CntIterations' iterations of the patch fitting
     * 
     * Time Complexity: O(CntIterations &times; (width &times; height &times; log<sup>2</sup>(width &times; height )))
     * 
     * @param file_name file name of the png image from which the patch will be copied
     * @param CntIterations number of iterations
     */
    void patchFitting(const std::string &file_name, int CntIterations = 100000);

    /**
     * @brief Chooses the format of the new patch from the inputImg at this position
     * 
     * Time Complexity: O(width &times; height &times; log<sup>2</sup>(width &times; height ))
     * 
     * @param heightOffset height offset of thhe position
     * @param widthOffset width offset of thhe position
     * @param inputImg png::image object from which the patch will be copied
     */
    void blending(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);

    /**
     * @brief Chooses the format of the new patch from the png image with this file name at this position
     *
     * Time Complexity: O(width &times; height &times; log<sup>2</sup>(width &times; height ))
     * 
     * @param heightOffset height offset of thhe position
     * @param widthOffset width offset of thhe position
     * @param file_name file name of the png image from which the patch will be copied
     */
    void blending(int heightOffset, int widthOffset, const std::string &file_name);
    
    /**
     * @brief Renders the constructed texture image
     * 
     * Time Complexity: linear on the number of pixels of the output image
     * 
     * @param file_name file name of the png image on which the texture will be rendered
     */
    void render(const std::string &file_name);
private:
    const uint64_t rngSeed;
    std::mt19937_64 rng;
    /// Enum of the status of each pixel on the output image
    enum PixelStatusEnum{
        colored, /// at least one pixel from a patch has been copied in this pixel
        intersection, /// this pixel is in the intersection from the already updated pixels and a pixel of the new rectangle
        newcolor, /// this pixel should be changed to a pixel from the new patch
        notcolored // no pixel from a patch has been copied in this pixel
    };
    // Enum of the type of the edges
    enum edgeType{
        originalGraph, // edge to the original graph
        copyGraph, // edge to the copy graph (to deal with the cut cycles)
        invalid // invalid edge
    };
    // Png image that will be construct
    png::image<png::rgb_pixel> outputImg;
    // width of output image
    const int imgWidth;
    // height of output image
    const int imgHeight;
    // pixel of color status, may be useful to change to a counter of the number of improvements of each pixel in some implementations of matching
    std::vector<std::vector<PixelStatusEnum>> pixelColorStatus;
    static constexpr long double inftyCost = 10000000;
    //pixels adjacent (ccw)
    static constexpr std::array<std::pair<int, int>, 4> directions = {{
        {-1, 0},    //    |0|
        { 0,-1},    //  |1|x|3|
        { 1, 0},    //    |2|
        { 0, 1}     //
    }};
    // identifiers of each vertex of square in grid (ccw)
    static constexpr std::array<std::pair<int, int>, 4> primalToDual = {{
        { 0, 0},    //  0_3
        { 1, 0},    //  | |
        { 1, 1},    //  1_2
        { 0, 1}     //
    }};
    // identifiers of each square adjacent to a vertex in grid (ccw)
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

    //Case 1 auxiliar methods
    std::pair<std::pair<int, int>, std::pair<int, int> > findSTInIntersectionCase1(Intersection &inter, int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    std::vector<Intersection> findIntersections(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    void markMinABCut(std::pair<int, int> S, std::pair<int, int> T, const ImageTexture::Intersection &inter, int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg);
    //MarkMinABCut auxiliar methods
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
