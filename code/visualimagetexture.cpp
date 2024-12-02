/**
 * @file visualimagetexture.cpp
 * @author Letícia Freire Carvalho de Sousa
 * @brief A visual implementation of ImageTexture.hpp
 * @version 0.1
 * @date 2024-10-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "imagetexture.hpp"
#define M_ASSERT(msg, expr) assert(( (void)(msg), (expr) ))

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
    pixelColorStatus(_img.get_height(), std::vector<PixelStatusEnum>(_img.get_width(), PixelStatusEnum::notcolored)),
    inSubgraph(imgHeight + 1, std::vector<bool>(imgWidth + 1)),
    edgesCosts(imgHeight + 1, std::vector<std::array<long double, 4>>(imgWidth + 1)),    
    dist(imgHeight + 1, std::vector<long double>(imgWidth + 1)),
    vis(imgHeight + 1, std::vector<bool>(imgWidth + 1)),
    parent(imgHeight + 1, std::vector<int>(imgWidth + 1, -1)),
    isT(imgHeight + 1, std::vector<bool>(imgWidth+1, false)),
    isS(imgHeight + 1, std::vector<bool>(imgWidth+1, false)),
    inS(imgHeight + 1, std::vector<bool>(imgWidth + 1)),
    inStPath(imgHeight + 1, std::vector<int>(imgWidth + 1, -1)),
    edgeTo({
        std::vector<std::vector<std::array<int, 4>>>(imgHeight + 1, std::vector<std::array<int, 4>>(imgWidth + 1, edgesToOriginalGraph)),
         std::vector<std::vector<std::array<int, 4>>>(imgHeight + 1, std::vector<std::array<int, 4>>(imgWidth + 1, edgesToOriginalGraph))
    }),
    distCase2({
        std::vector<std::vector<long double>>(imgHeight + 1, std::vector<long double>(imgWidth + 1, 0)),
        std::vector<std::vector<long double>>(imgHeight + 1, std::vector<long double>(imgWidth + 1, 0))
    }),
    visCase2({
        std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1, 0)),
        std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1, 0))
    }),
    seenCase2({
        std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1, 0)),
        std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1, 0))
    }),    
    parentCase2({
        std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1, -1)),
        std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1, -1))
    } )  
    {
}
ImageTexture::ImageTexture(int width, int height) 
    :  
    rngSeed(std::chrono::steady_clock::now().time_since_epoch().count()),
    rng(rngSeed),
    outputImg(width, height), 
    imgWidth(width),
    imgHeight(height), 
    pixelColorStatus(height, std::vector<PixelStatusEnum>(width, PixelStatusEnum::notcolored)),
    inSubgraph(imgHeight + 1, std::vector<bool>(imgWidth + 1)),
    edgesCosts(imgHeight + 1, std::vector<std::array<long double, 4>>(imgWidth + 1)),    
    dist(imgHeight + 1, std::vector<long double>(imgWidth + 1)),
    vis(imgHeight + 1, std::vector<bool>(imgWidth + 1)),
    parent(imgHeight + 1, std::vector<int>(imgWidth + 1, -1)),
    validEdge(imgHeight + 1, std::vector<std::array<bool, 4>>(imgWidth + 1, {true,true,true,true})),
    isT(imgHeight + 1, std::vector<bool>(imgWidth+1, false)),
    isS(imgHeight + 1, std::vector<bool>(imgWidth+1, false)),
    inS(imgHeight + 1, std::vector<bool>(imgWidth + 1)),
    inStPath(imgHeight + 1, std::vector<int>(imgWidth + 1, -1)),
    edgeTo({
        std::vector<std::vector<std::array<int, 4>>>(imgHeight + 1, std::vector<std::array<int, 4>>(imgWidth + 1, edgesToOriginalGraph)),
        std::vector<std::vector<std::array<int, 4>>>(imgHeight + 1, std::vector<std::array<int, 4>>(imgWidth + 1, edgesToOriginalGraph))
    }),
    distCase2({
        std::vector<std::vector<long double>>(imgHeight + 1, std::vector<long double>(imgWidth + 1, 0)),
        std::vector<std::vector<long double>>(imgHeight + 1, std::vector<long double>(imgWidth + 1, 0))
    }),
    visCase2({
        std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1, 0)),
        std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1, 0))
    }),
    seenCase2({
        std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1, 0)),
        std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1, 0))
    }),    
    parentCase2({
        std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1, -1)),
        std::vector<std::vector<int>>(imgHeight + 1, std::vector<int>(imgWidth + 1, -1))
    } )
        {    
}

/*
Public Functions
*/
void ImageTexture::render(const std::string &file_name){
    outputImg.write(file_name);
}
void ImageTexture::patchFitting(const png::image<png::rgb_pixel> &inputImg, int CntIterations){
    for(int i = 0 ; i < CntIterations; i++)
        patchFittingIteration(inputImg);
}
void ImageTexture::patchFitting(const std::string &file_name, int CntIterations){
    ImageTexture::patchFitting(png::image<png::rgb_pixel>(file_name));
}
void ImageTexture::patchFittingIteration(const png::image<png::rgb_pixel> &inputImg){
    const auto [heightOffset, widthOffset] = matching(inputImg);
    std::cout<<"Matching "<<heightOffset<<" "<<widthOffset<<"\n";
    if(isFirstPatch(heightOffset, widthOffset, inputImg)){
        copyFirstPatch(heightOffset, widthOffset, inputImg);
    }else
        blending(heightOffset, widthOffset, inputImg);
}
void ImageTexture::patchFittingIteration(const std::string &file_name){
    png::image<png::rgb_pixel> input_file;
    try{
        input_file = png::image<png::rgb_pixel>(file_name);
    } catch(...){
        std::cout<<"Invalid name for input file"<<std::endl;
        return;
    }
    patchFittingIteration(input_file);
}
void ImageTexture::blending(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    M_ASSERT("the new rectangle can't be all outside the output image",-(int) inputImg.get_height() + 1 <= heightOffset && heightOffset <= imgHeight-1);
    M_ASSERT("the new rectangle can't be all outside the output image",-(int) inputImg.get_width() + 1 <= widthOffset && widthOffset <= imgWidth-1);
    if(this->stPlanarGraph(heightOffset, widthOffset, inputImg)){
        this->blendingCase1(heightOffset, widthOffset, inputImg);
    }
    else{
        this->blendingCase2(heightOffset, widthOffset, inputImg);
    }
}
void ImageTexture::blending(int heightOffset, int widthOffset, const std::string &file_name){
    png::image<png::rgb_pixel> input_file;
    try{
        input_file = png::image<png::rgb_pixel>(file_name);
    } catch(...){
        std::cout<<"Invalid name for input file"<<std::endl;
        return;
    }
    blending(heightOffset, widthOffset, input_file);
}

/*
Private Functions
*/

// Main Private Functions

/**
 * @brief chooses matching position randomly
 * 
 * @param inputImg 
 * @return std::pair<int, int> 
 */
std::pair<int, int> ImageTexture::matching(const png::image<png::rgb_pixel> &inputImg){
    static std::uniform_int_distribution<int> nextHeight(-(int) inputImg.get_height() + 1, imgHeight-1);
    static std::uniform_int_distribution<int> nextWidth(-(int) inputImg.get_width() + 1, imgWidth-1);
    return {nextHeight(rng), nextWidth(rng)};
}

/**
 * @brief tests if the input image has any intersection with existing patches
 * 
 * @param heightOffset height position of the upper left corner of the input image on the output image
 * @param widthOffset width position of the upper left corner of the input image on the output image
 * @param inputImg png::image object from which the patch will be copied 
 * 
 * Time complexity: linear on the number of pixels of the input image
 * 
 * @return bool
 */
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
/**
 * @brief copies the full input image on the output image in this position
 * 
 * @param heightOffset height position of the upper left corner of the input image on the output image
 * @param widthOffset width position of the upper left corner of the input image on the output image
 * @param inputImg png::image object that will be copied
 *  
 * Time complexity: linear on the number of pixels of the input image
 * 
 * @return void
 */
void ImageTexture::copyFirstPatch(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
            if(a < 0 || b < 0)
                continue;
            this->pixelColorStatus[a][b] = PixelStatusEnum::newcolor;
        }  
    copyPixelsNewColor(heightOffset, widthOffset, inputImg);
}
/**
 * @brief tests if the 
 * 
 * @param heightOffset height position of the upper left corner of the input image on the output image
 * @param widthOffset width position of the upper left corner of the input image on the output image
 * @param inputImg png::image object that will be copied
 *  
 * Time complexity: linear on the number of pixels of the input image
 * 
 * @return void
 */
bool ImageTexture::stPlanarGraph(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    {//upper edge
        int upperEdgeHeight = std::max(0, heightOffset);
        for(int j = 0; j < (int) inputImg.get_width() && j + widthOffset < this->imgWidth; j++){
            if(j + widthOffset < 0)
                continue;
            if(this->pixelColorStatus[upperEdgeHeight][j + widthOffset] == this->PixelStatusEnum::notcolored)
                return true;
        }
    }
    {//left edge
        int leftEdgeWidth = std::max(0, widthOffset);
        for(int i = 0; i < (int) inputImg.get_height() && i + heightOffset < this->imgHeight; i++){
            if(i + heightOffset < 0)
                continue;
            if(this->pixelColorStatus[i + heightOffset][leftEdgeWidth] == this->PixelStatusEnum::notcolored)
                return true;
        }
    }
    {//lower edge
        int lowerEdgeHeight = std::min<int>(heightOffset + inputImg.get_height() - 1 , this->imgHeight - 1);
        for(int j = 0; j < (int) inputImg.get_width() && j + widthOffset < this->imgWidth; j++){
            if(j + widthOffset < 0)
                continue;
            if(this->pixelColorStatus[lowerEdgeHeight][j + widthOffset] == this->PixelStatusEnum::notcolored)
                return true;
        }
    }
    {//right edge
        int rightEdgeWidth = std::min<int>(widthOffset + inputImg.get_width() - 1, this->imgWidth - 1);
        for(int i = 0; i < (int) inputImg.get_height() && i + heightOffset < this->imgHeight; i++){
            if(i + heightOffset < 0)
                continue;
            if(this->pixelColorStatus[i + heightOffset][rightEdgeWidth] == this->PixelStatusEnum::notcolored)
                return true;
        }
    }
    return false;
}
void ImageTexture::blendingCase1(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    std::vector<Intersection> intersections = findIntersections(heightOffset, widthOffset, inputImg);
    for(auto &inter : intersections){
        auto [S, T] = findSTInIntersectionCase1(inter, heightOffset, widthOffset, inputImg);
        markMinABCut(S, T, inter, heightOffset, widthOffset, inputImg);
    }
    copyPixelsNewColor(heightOffset, widthOffset, inputImg);

    /*sanity test*/{
        for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
            for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
                if(a < 0 || b < 0)
                    continue;
                M_ASSERT("All pixels should be colored", pixelColorStatus[a][b] == PixelStatusEnum::colored);
            }
    }
}
void ImageTexture::blendingCase2(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    /*find intersection*/
    auto intersections = findIntersections(heightOffset, widthOffset, inputImg);
    assert(intersections.size() == 1);
    Intersection inter = intersections[0];
    
    /*mark cells in dual of intersection and mark edges costs*/
    std::vector<std::pair<int, int>> cellsInDual = markIntersectionCellsInDual(inter);
    markIntersectionEdgeCostsInDual(heightOffset, widthOffset, inputImg, cellsInDual);

    /*find ST path*/
    auto S = findSCase2(heightOffset, widthOffset, inputImg);
    if(S.empty()){ // degenerate case
        for(auto [x, y] : inter.interPixels)
            pixelColorStatus[x][y] = PixelStatusEnum::colored;
        for(auto [i,j]: cellsInDual)
            inSubgraph[i][j] = false;
        return;
    }
    auto T = dualBorder(heightOffset, widthOffset, inputImg);
    auto tsPath = findSTPath(S, T);
    

    // exit by right - original graph
    // enter by left - copy graph
    /*mark ST Path*/{
        M_ASSERT("tsPath lenght should be at least 2", int(tsPath.size()) >= 2);
        
        for(auto [x, y] : S)
            inS[x][y] = true;
        for(int i = 0; i < (int) tsPath.size(); i++){
            auto [x, y] = tsPath[i];
            inStPath[x][y] = i;
        }

        auto [secondX, secondY] = tsPath[tsPath.size() - 2];
        for(int i =0; i < int(tsPath.size()) - 1; i++){
            auto [x, y] = tsPath[i];
            auto [nextX, nextY] = tsPath[i+1];
            int d = parent[x][y];
            edgeTo[edgeType::copyGraph][nextX][nextY][revDir(d)] = edgeType::copyGraph;
            edgeTo[edgeType::copyGraph][x][y][d] = edgeType::copyGraph;
        }
        {// dealing with edges of T
            int d = prevDir( revDir(parent[secondX][secondY]) );
            auto [lastX, lastY] = tsPath.back();
            for(int i = 0; i < 3; i++){
                int nextX = lastX + directions[d].first, nextY = lastY + directions[d].second;
                if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
                    edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::invalid;
                    if(inS[nextX][nextY]) break;
                }
                d = prevDir(d);
            }
            d = nextDir( revDir(parent[secondX][secondY]) );
            for(int i = 0; i < 3; i++){
                int nextX = lastX + directions[d].first, nextY = lastY + directions[d].second;
                if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
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
                edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::copyGraph;
                edgeTo[edgeType::originalGraph][x][y][d] = edgeType::invalid;
            }
            d = prevDir(d);
            nextX = x + directions[d].first, nextY = y + directions[d].second;
            if((i > 0 && tsPath[i-1] == std::make_pair(nextX, nextY))|| (i == 0 && d == revDir(parent[x][y])))
                continue;
            if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
                edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::copyGraph;
                edgeTo[edgeType::originalGraph][x][y][d] = edgeType::invalid;
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
                edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::invalid;
            }
            d = nextDir(d);
            nextX = x + directions[d].first, nextY = y + directions[d].second;
            if((i > 0 && tsPath[i-1] == std::make_pair(nextX, nextY)) || (i == 0 && d == revDir(parent[x][y])))
                continue;
            if(insideDual(nextX, nextY) && inSubgraph[nextX][nextY]){
                edgeTo[edgeType::originalGraph][nextX][nextY][revDir(d)] = edgeType::invalid;
            }
        }
    }

    std::pair<long double, std::vector<std::array<int,3>>> minCut;
    { 
        int visited = 0;   
        minCut = minCutCycle(0, tsPath.size(), tsPath, visited);
        
        /*mark left and right of min cut*/
        markLeftOfMinCut(tsPath);
        /*mark right of min cut*/{    
            for(auto [i, j] : inter.interPixels)
                if(pixelColorStatus[i][j] == PixelStatusEnum::intersection){
                    pixelColorStatus[i][j] = PixelStatusEnum::newcolor;
                }
        }
        copyPixelsNewColor(heightOffset, widthOffset, inputImg, true);

        for(auto [i, j] : cellsInDual){
            distCase2[0][i][j] = 0;
            distCase2[1][i][j] = 0;
            visCase2[0][i][j] = 0;
            visCase2[1][i][j] = 0;
            seenCase2[0][i][j] = 0;
            seenCase2[1][i][j] = 0;
            parentCase2[0][i][j] = -1;
            parentCase2[1][i][j] = -1;
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

    /*unmark ST Path*/{
        for(auto [x, y] : tsPath)
            inStPath[x][y] = -1;
    }

    /*unmark cells in dual of intersection*/{
        for(auto [i,j] : cellsInDual){
            inSubgraph[i][j] = false;
            parent[i][j] = -1;
            vis[i][j] = false;
        }
    }
}

// Auxiliar Static Functions
long double ImageTexture::pow2(long double x){
    return x * x;
}
template<typename Pixel>
long double ImageTexture::calcCost(const Pixel &as, const Pixel &bs, const Pixel &at, const Pixel &bt){
    long double cost =
    sqrtl(pow2((int)as.red-bs.red) + pow2((int)as.green-bs.green) + pow2((int)as.blue-bs.blue)) +
    sqrtl(pow2((int)at.red-bt.red) + pow2((int)at.green-bt.green) + pow2((int)at.blue-bt.blue));
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
void ImageTexture::copyPixelsNewColor(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg, bool case2){
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
            if(a < 0 || b < 0)
                continue;
            if(pixelColorStatus[a][b] == PixelStatusEnum::newcolor){
                outputImg[a][b] = png::rgb_pixel(0,0,155);
                if(case2)
                    outputImg[a][b] = png::rgb_pixel(155,0,0);
            }
        }
    render("../output_images/output.png");
    usleep(800000);
    
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
            if(a < 0 || b < 0)
                continue;
            if(pixelColorStatus[a][b] == PixelStatusEnum::newcolor){
                outputImg[a][b] = inputImg[i][j];
                pixelColorStatus[a][b] = PixelStatusEnum::colored;
            }
        }
    render("../output_images/output.png");
    usleep(800000);
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
// Auxiliar class
ImageTexture::Intersection::Intersection(const std::vector<std::pair<int,int>> &pixels) : interPixels(pixels){};
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
    
    M_ASSERT("findSTInIntersectionCase1 should always find S", (S) != (std::pair<int, int>{-1,-1}));
    M_ASSERT("findSTInIntersectionCase1 should always find T", (T) != (std::pair<int, int>{-1,-1}));
    M_ASSERT("findSTInIntersectionCase1 should find different S and T", (S) != (T));
    return {S, T};
}
std::vector<ImageTexture::Intersection> ImageTexture::findIntersections(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg){
    std::vector<Intersection> intersectionsList;
    for(int i = 0, a = i + heightOffset; i < (int) inputImg.get_height() && a < this->imgHeight; i++, a++)
        for(int j = 0, b = j + widthOffset; j < (int) inputImg.get_width() && b < this->imgWidth; j++, b++){
            if(a < 0 || b < 0)
                continue;
            if(pixelColorStatus[a][b] == PixelStatusEnum::colored){
                std::vector<std::pair<int,int>> interPixels;
                pixelColorStatus[a][b] = PixelStatusEnum::intersection;
                interPixels.emplace_back(a,b);
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
    /*mark cells in dual of intersection and mark edges costs*/
    
    std::vector<std::pair<int, int>> inDual = markIntersectionCellsInDual(inter);
    markIntersectionEdgeCostsInDual(heightOffset, widthOffset, inputImg, inDual);
    
    /*find min cut (min s-t path)*/
    auto tsPath = findSTPath({S}, {T});
    
    /*mark edges on the path*/
    M_ASSERT("T should always be visited, intersection is connected", (vis[T.first][T.second]));
    
    /*mark ST path*/{
        int lastD = -1;
        for(auto [curI, curJ] : tsPath){
            if(lastD >= 0){
                validEdge[curI][curJ][lastD] = false;
            }
            int d = parent[curI][curJ];
            if(d >= 0){
                validEdge[curI][curJ][d] = false;
                lastD = revDir(d);
            }
        }
    }    
    
    /*mark left and right of min cut*/
    markLeftOfMinCut(tsPath);
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
std::vector<std::pair<int, int>> ImageTexture::markIntersectionCellsInDual(const Intersection &inter){
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
void ImageTexture::markIntersectionEdgeCostsInDual(int heightOffset, int widthOffset, const png::image<png::rgb_pixel> &inputImg, const std::vector<std::pair<int, int>> &inDual){
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

std::vector<std::pair<int,int>> ImageTexture::findSTPath(const std::vector<std::pair<int,int>> &S, const std::vector<std::pair<int,int>> &T){
    for(auto [h, w] : T){
        isT[h][w] = true;
        M_ASSERT("T should be in subgraph", inSubgraph[h][w]);
    }
    using qtype = std::tuple<long double, int, int>;
    std::priority_queue<qtype, std::vector<qtype>, std::greater<qtype>> Q;
    for(auto [h, w] : S){
        M_ASSERT("S should be in subgraph", inSubgraph[h][w]);
        isS[h][w] = true;
        parent[h][w] = -2;
        dist[h][w] = 0;
        Q.emplace(0, h, w);
    }
    ////std::cout<<"START DIJKSTRA"<<std::endl;
    std::vector<std::pair<int, int>> path;
    while(!Q.empty()){
        long double pathCost;
        int i, j;
        std::tie(pathCost, i, j) = Q.top();
        Q.pop();
        assert(i < (int) vis.size());
        assert(j < (int) vis[i].size());
        if(vis[i][j])
            continue;
        
        vis[i][j] = true;
        if(isT[i][j]){
            path = {{i,j}};
            break;
        }
        for(int d = 0; d < int(directions.size()); d++){
            int nextI = i + directions[d].first, nextJ = j + directions[d].second;
            if(insideDual(nextI, nextJ) && inSubgraph[nextI][nextJ] && !vis[nextI][nextJ]){
                long double curCost = edgesCosts[i][j][d];
                if(parent[nextI][nextJ] == -1 || pathCost + curCost < dist[nextI][nextJ]){
                    parent[nextI][nextJ] = revDir(d);
                    dist[nextI][nextJ] = pathCost + curCost;
                    Q.emplace(dist[nextI][nextJ], nextI, nextJ);
                }
            }
        }
    }
    // only path is direct path between S and T
    if(!path.empty()){
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
    M_ASSERT("path is empty!", !path.empty());
    int curI, curJ;
    std::tie(curI, curJ) = path[0];
    while(parent[curI][curJ] != -2){
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

void ImageTexture::markLeftOfMinCut(const std::vector<std::pair<int, int>> &cut){
    for(auto [i, j] : cut){
        int d = parent[i][j];
        if(d < 0)
            break;
        assert(0 <= d && d < int(directions.size()));
        auto [curI, curJ] = std::make_pair(i + directions[d].first, j + directions[d].second);
        d = revDir(d);
        int firstI, firstJ;
        std::tie(firstI, firstJ) = std::make_pair(curI + dualToPrimal[d].first, curJ + dualToPrimal[d].second);
        //BFS Marking left
        if(insidePrimal(firstI, firstJ) && pixelColorStatus[firstI][firstJ] == PixelStatusEnum::intersection){
            std::vector<std::pair<int, int>> q = {{firstI, firstJ}};
            pixelColorStatus[firstI][firstJ] = PixelStatusEnum::colored;
            for(int front = 0; front < int(q.size()); front++){
                int fI = q[front].first, fJ = q[front].second;
                for(int dir = 0; dir < (int) directions.size(); dir++){
                    int nxtI = fI + directions[dir].first;
                    int nxtJ = fJ + directions[dir].second;
                    if(!insidePrimal(nxtI, nxtJ) || pixelColorStatus[nxtI][nxtJ] != PixelStatusEnum::intersection)
                        continue;
                    int dualI = fI + primalToDual[dir].first;
                    int dualJ = fJ + primalToDual[dir].second;
                    assert(insideDual(dualI, dualJ));
                    if(validEdge[dualI][dualJ][prevDir(dir)]){
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
        if(upperEdgeHeight >= lowerEdgeHeight) // degenerate case
            return {};
        if(leftEdgeWidth >= rightEdgeWidth) // degenerate case
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

std::pair<long double, std::vector<std::array<int,3>>> ImageTexture::findMinFCycle(const std::pair<int,int> &F, int visited){
    std::array<int, 3> S = {0, F.first, F.second};
    std::array<int, 3> T = {1, F.first, F.second};
    using qtype = std::tuple<long double, int, int, int>;
    std::priority_queue<qtype, std::vector<qtype>, std::greater<qtype>> Q;
    parentCase2[S[0]][S[1]][S[2]] = -2;
    distCase2[S[0]][S[1]][S[2]] = 0;
    Q.emplace(0, S[0], S[1], S[2]);
    std::vector<std::array<int,3>> path;
    while(!Q.empty()){
        long double pathCost;
        int g, i, j;
        std::tie(pathCost, g, i, j) = Q.top();
        Q.pop();
        
        assert(i < (int) visCase2[0].size());
        assert(j < (int) visCase2[0][i].size());
        if(visCase2[g][i][j] == visited)
            continue;
        
        visCase2[g][i][j] = visited;
        if(T == std::array{g,i,j}){
            path = {{g,i,j}};
            break;
        }
        for(int d = 0; d < int(directions.size()); d++){
            int nextI = i + directions[d].first, nextJ = j + directions[d].second;
            int nextG = edgeTo[g][i][j][d];
            if(nextG == 2)
                continue;
            if(insideDual(nextI, nextJ) && inSubgraph[nextI][nextJ] && visCase2[nextG][nextI][nextJ] != visited){
                long double curCost = edgesCosts[i][j][d];
                if(seenCase2[nextG][nextI][nextJ] != visited || pathCost + curCost < distCase2[nextG][nextI][nextJ]){
                    seenCase2[nextG][nextI][nextJ] = visited;
                    parentCase2[nextG][nextI][nextJ] = revDir(d)*10+g;
                    distCase2[nextG][nextI][nextJ] = pathCost + curCost;
                    Q.emplace(distCase2[nextG][nextI][nextJ], nextG, nextI, nextJ);
                }
            }
        }
    }
    M_ASSERT("path is empty!", !path.empty());
    int curG = T[0], curI, curJ;
    std::tie(curG, curI, curJ) = std::tuple_cat(path[0]);
    while(parentCase2[curG][curI][curJ] != -2){
        int d = parentCase2[curG][curI][curJ]/10;
        curG = parentCase2[curG][curI][curJ]%10;
        M_ASSERT("direction must be valid!", 0 <= d && d < int(directions.size()));
        std::tie(curI, curJ) = std::make_tuple(curI + directions[d].first, curJ + directions[d].second);
        path.push_back({curG, curI, curJ});
    }

    return {distCase2[T[0]][T[1]][T[2]], path};
}


std::pair<long double, std::vector<std::array<int,3>>> ImageTexture::minCutCycle(int left, int right, const std::vector<std::pair<int, int>> &stPath, int &visited){
    visited++;
    static std::vector<std::vector<int>> inCutCycle(imgHeight + 1, std::vector<int>(imgWidth + 1));
    
    int f_mid = (left + right) / 2;
    
    auto curCutCycle = findMinFCycle(stPath[f_mid], visited);
    
    auto answerCut = curCutCycle;

    std::vector<int> parentsOfCutCycle(curCutCycle.second.size());
    for(int i = 0; i < (int) curCutCycle.second.size(); i++){
        auto [g, x, y] = curCutCycle.second[i];
        parentsOfCutCycle[i] = parentCase2[g][x][y];
        inCutCycle[x][y]++;
    }  
    
    if(left < f_mid){
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
                oldEdgeValues.push_back({edgeType::originalGraph,x,y,d,edgeTo[edgeType::originalGraph][x][y][d]});
                edgeTo[edgeType::originalGraph][x][y][d] = edgeType::invalid;
                oldEdgeValues.push_back({edgeType::copyGraph,x,y,d,edgeTo[edgeType::copyGraph][x][y][d]});
                edgeTo[edgeType::copyGraph][x][y][d] = edgeType::invalid;
            }
        }
        auto leftCutCycle = minCutCycle(left, f_mid, stPath, visited);
        reverse(oldEdgeValues.begin(), oldEdgeValues.end());
        for(auto [g,x,y,dir,value] : oldEdgeValues){
            edgeTo[g][x][y][dir] = value;
        }
        answerCut = min(answerCut, leftCutCycle);
    }
    //right
    if(f_mid + 1 < right){
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
                oldEdgeValues.push_back({edgeType::originalGraph,x,y,d,edgeTo[edgeType::originalGraph][x][y][d]});
                edgeTo[edgeType::originalGraph][x][y][d] = edgeType::invalid;
                oldEdgeValues.push_back({edgeType::copyGraph,x,y,d,edgeTo[edgeType::copyGraph][x][y][d]});
                edgeTo[edgeType::copyGraph][x][y][d] = edgeType::invalid;
            }
        }
        auto rightCutCycle = minCutCycle(f_mid + 1, right, stPath, visited);
        reverse(oldEdgeValues.begin(), oldEdgeValues.end());
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