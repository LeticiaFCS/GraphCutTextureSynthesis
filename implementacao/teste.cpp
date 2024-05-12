#include <png++/png.hpp>
#include <math.h>
#include <array>
#include <queue>
#include <tuple>
#include <unistd.h>  //just for debug
#include <cstdlib> // just for debug
#include <map>

using namespace std;

const int64_t inf = numeric_limits<int64_t>::max() / 4;

enum Pixel_State{
    colored, intersection, newcolor, notcolored
};

int64_t pow2(int64_t x){
    return x * x;
}

template<typename Pixel>
long double calc_cost(const Pixel &as, const Pixel &bs, const Pixel &at, const Pixel &bt){
    return
    sqrtl(pow2(as.red-bs.red) + pow2(as.green-bs.green) + pow2(as.blue-bs.blue)) +
    sqrtl(pow2(at.red-bt.red) + pow2(at.green-bt.green) + pow2(at.blue-bt.blue));
}

void init_texture(png::image<png::rgb_pixel> &input_img, png::image<png::rgb_pixel> &output_img, vector<vector<Pixel_State>> &colored){
    // for(int i = 0; i < input_img.get_height(); i++)
    //     for(int j = 0; j < input_img.get_width(); j++){
    //         output_img[i][j] = input_img[i][j];
    //         colored[i][j] = Pixel_State::colored;
    //     }

    
    for(int i = 0; i < input_img.get_height() && i < output_img.get_height(); i++)
        for(int j = 0; j < input_img.get_width() && j < output_img.get_width(); j++){
            colored[j][i] = Pixel_State::colored;
            output_img[j][i] = input_img[j][i];
        }
}
template<typename T>
void clean_graph(vector<vector<T>> &g){
    using namespace std;
    for(int i = 0; i < (int)g.size(); i++){
        sort(g[i].begin(), g[i].end());
        g[i].erase(unique(g[i].begin(), g[i].end()), g[i].end());
        // cout<<i<<":  ";
        // for(auto [c, j] : g[i]) cout<<j<<", ";
        // cout<<endl;
    }
}

template<typename C>
vector<int> min_path(vector<vector<pair<C, int>>> &g, vector<bool> &vis, vector<bool> &inPath, const int S, const int T){
    using namespace std;
    const int N = (int) g.size();
    vector<C> dist(N, numeric_limits<C>::max());
    vector<int> parent(N, -1);
    priority_queue<pair<C, int>, vector<pair<C, int>>, greater<pair<C, int>>> q;
    parent[S] = S;
    dist[S] = 0;
    q.emplace(dist[S], S);
    while(!q.empty()){
        auto [d, v] = q.top();
        q.pop();
        if(vis[v]) continue;
        vis[v] = true;
        if(v == T){
            break;
        }
        for(auto [cost, nxt] : g[v]){
            if(vis[nxt]) continue;
            if(dist[v] + cost < dist[nxt]){
                parent[nxt] = v;
                dist[nxt] = dist[v] + cost;
                q.emplace(dist[nxt], nxt);
            }
        } 
    }
    cout<<"S "<<S<<" T "<<T<<endl;
    assert(vis[T]);
    int current = T;
    vector<int> path;
    while(current != parent[current]){
        path.push_back(current);
        //cout<<" INPATH "<<current<<endl;
        inPath[current] = true;
        current = parent[current];
    }
    path.push_back(S);
    //reverse(path.begin(), path.end());
    //cout<<endl;
    return path;
}

template<typename C>
void copy_patch(const vector<vector<pair<C, int>>> &g, const int B, 
                const vector<pair<int, int>> &positions, const vector<bool> &vis,
                png::image<png::rgb_pixel> &input_img, png::image<png::rgb_pixel> &output_img){
    

}

void blending(int h, int w, png::image<png::rgb_pixel> &input_img, png::image<png::rgb_pixel> &output_img, vector<vector<Pixel_State>> &colored){
    using namespace png;
    cout<<"START BLENDING\n";
    static vector<vector<int>> ids(output_img.get_height(), vector<int>(output_img.get_width()));
    const int source = 0, sink = 1;
    vector<pair<int, int>> positions;
    vector<png::rgb_pixel> a, b;
    for(int i = 0, y = i + h; i < input_img.get_height() && y < output_img.get_height(); i++, y++)
        for(int j = 0, x = j + w; j < input_img.get_width() && x < output_img.get_width(); j++, x++)
            if(colored[y][x] == Pixel_State::colored){
                ids[y][x] = positions.size();
                positions.push_back({y, x});
                a.push_back(output_img[y][x]);
                b.push_back(input_img[i][j]);
                colored[y][x] = Pixel_State::intersection;
            } else
                colored[y][x] = Pixel_State::newcolor;
    auto valid = [&](int y, int x) -> bool {
        return 0 <= y && y < output_img.get_height() && 0 <= x && x < output_img.get_width();
    };
    vector<pair<int, int>> directions = {{-1,0}, {0,-1}, {1,0}, {0,1}};
    vector<array<int, 4>> dual_ids(positions.size(), {-1});
    map<pair<int, int>, int> pixel_left_of_edge;
    vector<vector<pair<int64_t, int>>> dual_graph; dual_graph.reserve(positions.size());
    vector<bool> adjacent_new_color; adjacent_new_color.reserve(positions.size());
    vector<bool> adjacent_old_color; adjacent_old_color.reserve(positions.size());
    int dual_n = 0;
    for(int p = 0; p < positions.size(); p++){
        const auto [y, x] = positions[p];

        int y_nei0 = y + directions[0].first, x_nei0 = x + directions[0].second;
        bool valid_nei0 = (valid(y_nei0, x_nei0) && colored[y_nei0][x_nei0] == Pixel_State::intersection);

        if(valid_nei0){
            int p_nei0 = ids[y_nei0][x_nei0];
            dual_ids[p][0] = dual_ids[p_nei0][3];
            dual_ids[p][1] = dual_ids[p_nei0][2];
        } else {
            dual_ids[p][0] = dual_n++;
            dual_graph.push_back({});
            adjacent_new_color.push_back(false);
            adjacent_old_color.push_back(false);
        }
        
        int y_nei1 = y + directions[1].first, x_nei1 = x + directions[1].second;
        bool valid_nei1 = (valid(y_nei1, x_nei1) && colored[y_nei1][x_nei1] == Pixel_State::intersection);

        if(!valid_nei0 && !valid_nei1){
            dual_ids[p][1] = dual_n++;
            dual_graph.push_back({});
            adjacent_new_color.push_back(false);
            adjacent_old_color.push_back(false);
        }

        if(valid_nei1){
            int p_nei1 = ids[y_nei1][x_nei1];
            dual_ids[p][1] = dual_ids[p_nei1][0];
            dual_ids[p][2] = dual_ids[p_nei1][3];
        } else {
            dual_ids[p][2] = dual_n++;
            dual_graph.push_back({});
            adjacent_new_color.push_back(false);
            adjacent_old_color.push_back(false);
        }
        
        dual_ids[p][3] = dual_n++;
        dual_graph.push_back({});
        adjacent_new_color.push_back(false);
        adjacent_old_color.push_back(false);
        for(int i = 0, j = 1; i < 4; i++, j = (j+1)%4){
            int a = dual_ids[p][i];
            int b = dual_ids[p][j];
            pixel_left_of_edge[{a, b}] = p;
            int y_nei = y + directions[i].first, x_nei = x + directions[i].second;
            if(valid(y_nei, x_nei)){
                if(colored[y_nei][x_nei] == Pixel_State::intersection){
                    int64_t cost = 1; // TO DO - change to real cost
                    dual_graph[a].emplace_back(cost, b);
                    dual_graph[b].emplace_back(cost, a);
                } else {
                    dual_graph[a].emplace_back(inf, b);
                    dual_graph[b].emplace_back(inf, a);
                    if(colored[y_nei][x_nei] == Pixel_State::colored){
                        adjacent_old_color[a] = true;
                        adjacent_old_color[b] = true;
                    }
                    else if(colored[y_nei][x_nei] == Pixel_State::newcolor){
                        adjacent_new_color[a] = true;
                        adjacent_new_color[b] = true;
                    }
                }
            } else {
                dual_graph[a].emplace_back(inf, b);
                dual_graph[b].emplace_back(inf, a);
            }
        }
        for(int i = 0; i < 4; i++){
            int a = dual_ids[p][i];
            //cout<<dual_ids[p][i]<<" "<<adjacent_old_color[a]<<" "<<adjacent_new_color[a]<<"\n";
        }   
        //cout<<y<<" "<<x<<": \n\t";
        for(int i = 0; i < 4; i++){
            //cout<<dual_ids[p][i]<<", ";
        }
        //cout<<"\n";
    }
    clean_graph(dual_graph);
    assert(dual_graph.size() == dual_n);
    png::rgb_pixel rand_pixel(rand()%256, rand()%256, rand()%256);
    //empty intersection --> first patch
    if(dual_n == 0){
        cout<<"FIRST PATCH"<<endl;
        //copy image
        for(int i = 0, y = i + h; i < input_img.get_height() && y < output_img.get_height(); i++, y++)
            for(int j = 0, x = j + w; j < input_img.get_width() && x < output_img.get_width(); j++, x++){
                colored[y][x] = Pixel_State::colored;
                output_img[y][x] = input_img[i][j];
                //output_img[y][x] = rand_pixel;
            }
        return;
    }
    cout<<"INTERSECTION"<<endl;
    vector<bool> visited(dual_n), inPath(dual_n);
    vector<int> graph_component(dual_n, -1);
    int count_components = 0;
    for(int v = 0; v < dual_n; v++){
        if(graph_component[v] != -1)
            continue;
        int A = -1, B = -1;
        auto dfs_mark = [&](int r, auto &&self) -> void {
            if(adjacent_old_color[r] && (A == -1))
                A = r;
            else if(adjacent_new_color[r] && (B == -1))
                B = r;
            graph_component[r] = count_components; 
            for(auto [cost, neigh] : dual_graph[r]){
                if(graph_component[neigh] == -1)
                    self(neigh, self);
            }           
        };
        dfs_mark(v, dfs_mark);
        count_components++;
        cout<<"A "<<A<<" B "<<B<<endl;
        assert(A != -1);
        assert(B != -1);
        // unsurrounded region
        if(A != -1 && B != -1){ 
            cout<<"UNSURROUNDED"<<endl;
            auto minpath = min_path(dual_graph, visited, inPath, A, B);
            cout<<"Path:\n";
            for(int i = 0; i < int(minpath.size()) - 1; i++){
                int a = minpath[i], b = minpath[i+1];
                int p = pixel_left_of_edge[{a,b}];
                auto [y,x] = positions[p];
                if(valid(y, x) && colored[y][x] == Pixel_State::intersection){
                    auto dfs_color = [&](int y, int x, auto &&self) -> void {
                        int p = ids[y][x];
                        //colored[y][x] = Pixel_State::newcolor;
                        {
                            colored[y][x] = Pixel_State::colored;
                            output_img[y][x] = output_img[y][x] = rand_pixel;
                        }
                        for(int d = 0; d < 4; d++){
                            int a = dual_ids[p][d];
                            int b = dual_ids[p][(d+1)%4];
                            if(inPath[a] && inPath[b])
                                continue;
                            cout<<"CHECK INTERSECTION\n";
                            int nei_y = y + directions[d].first;
                            int nei_x = x + directions[d].second;
                            if(valid(nei_y, nei_x) && colored[nei_y][nei_x] == Pixel_State::intersection){
                                cout<<"IN\n";
                                self(nei_y, nei_x, self);
                            }
                        }
                    };
                    dfs_color(y, x, dfs_color);
                }
            }
            cout<<endl;
        } 
        // surrounded region
        else {
            cout<<"SURROUNDED"<<endl;
            // TO-DO!
        }
    }
    for(int i = 0, y = i + h; i < input_img.get_height() && y < output_img.get_height(); i++, y++)
        for(int j = 0, x = j + w; j < input_img.get_width() && x < output_img.get_width(); j++, x++){
            if(colored[y][x] == Pixel_State::newcolor){
                output_img[y][x] = input_img[i][j];
                //output_img[y][x] = rand_pixel;
            }
            colored[y][x] = Pixel_State::colored;
        }


}


void create_texture(const unsigned int output_height, const unsigned int output_width, const string &input_f, const string &output_f){
    using namespace png;
    image<rgb_pixel> input_img(input_f);
    assert(input_img.get_height() == 32);
    assert(input_img.get_width() == 32);
    //image<rgb_pixel> output_img(output_height, output_width); // old
    image<rgb_pixel> output_img(output_width, output_height);
    assert(output_img.get_height() == output_height);
    assert(output_img.get_width() == output_width);
    vector<vector<Pixel_State>> colored(output_height, vector<Pixel_State>(output_width, Pixel_State::notcolored));
    // init_texture(input_img, output_img, colored);
    // output_img.write(output_f);
    int patches = 0;
    for(int h = 0; h < output_height; h += input_img.get_height() - 8){
        for(int w = 0; w < output_width; w += input_img.get_width() - 8){
            usleep(40000);
            patches += 1;
            cout<<"Case "<<patches<<endl;
            cout<<h<<" "<<w<<endl;
            blending(h, w, input_img, output_img, colored);
            output_img.write(output_f);
        }
    }
    output_img.write(output_f);
}

int main() { 
    create_texture(500, 400, "input/areia_da_praia.png", "output/areia_da_praia.png");

 
    return 0;  
} 
