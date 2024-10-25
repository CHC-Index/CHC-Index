//This file contains code for core decomposition and truss decomposition.
#ifndef CNNS_DECOMPOSITION_H
#define CNNS_DECOMPOSITION_H

#include <iostream>
#include <vector>
#include <queue>
#include <cassert>
#include <iomanip>
#include <phmap.h>
#include <string>
#include <algorithm>
#include "basic_define.h"

using phmap::parallel_flat_hash_map;
using phmap::flat_hash_map;
namespace chci {
    template<class T>
    class decomposition {
    public:

        struct cmp {
            constexpr bool
            operator()(std::pair<CDEGREE , T> const &a,
                       std::pair<CDEGREE , T> const &b) const noexcept {
                return a.first > b.first;
            }
        };
        using minHeap = std::priority_queue<std::pair<CDEGREE, T>, std::vector<std::pair<CDEGREE, T>>, cmp>;
        decomposition() = default;

        decomposition(std::vector<std::vector<T> > &adj, T ntotal):
            ori_adj(adj), ntotal_(ntotal), core_list(ntotal, -1)
            {
            max_Deg = 0;
            for (auto i = ori_adj.begin(); i != ori_adj.end(); i++){
                if ((*i).size() > max_Deg)
                    max_Deg = i->size();
                deg_list.emplace_back(i->size());
            }
            assert(deg_list.size() == ntotal_);

            vert = std::vector<CNODEID>(ntotal_, -1);
            pos = std::vector<CNODEID>(ntotal_, 0);
            bin = std::vector<CNODEID>(max_Deg+1, 0);
            deg = deg_list;
            for(T i = 0; i <= max_Deg; ++i){
                T cnt = 0;
                for (T j = 0; j < ntotal_; j++){
                    if (deg[j] == i){
                        vert[bin[i]+cnt] = j;
                        pos[j] = bin[i] + cnt ;
                        cnt++;
                    }
                }
                if (i != max_Deg)
                    bin[i+1] = bin[i] + cnt;

            }
        }
        ~decomposition() = default;
          std::vector<CCORE> BZ_Decomp(){
            CNODEID du = -1;
            CNODEID pu = -1;
            CNODEID pw = -1;
            CNODEID w = -1;
            for (CNODEID i = 0; i < ntotal_ ; i++){
                if ( i % (ntotal_/10) == 0)
                    std::cout << "Core Decomposition done " << i/(ntotal_/10) <<"0% " << std::endl;
                auto v = vert[i];
                for (auto u : ori_adj[v]){
                    if (deg[u] > deg[v]){
                        du = deg[u];
                        pu = pos[u];
                        pw = bin[du];
                        w = vert[pw];

                        if (u != w){
                            pos[u] = pw;
                            pos[w] = pu;
                            vert[pu] = w;
                            vert[pw] = u;
                        }
                        bin[du] = bin[du] + 1;
                        deg[u] = deg[u] - 1;
                    }
                }
            }
            for (auto i = 0; i < ntotal_; i++){
                core_list[i] = static_cast<CCORE >(deg[i]);
            }
              return core_list;
        }
        std::vector<std::vector<T>> ori_adj;
        std::vector<CDEGREE> deg_list;
        std::vector<CCORE> core_list;
        T max_Deg;
        T ntotal_;
        std::vector<std::vector<T>> a;

        std::vector<CDEGREE> deg;
        std::vector<CNODEID> vert;
        std::vector<CNODEID> pos;
        std::vector<CNODEID> bin;
    };
    
    class truss_decomposition {
    public:
        typedef std::pair<CNODEID, CNODEID> vert_pair;
        
        std::vector<std::vector<CNODEID> > cur_adj;
        adj_t *porigin_adj;
        parallel_flat_hash_map<vert_pair, CEDGEID> edge_map;
        std::vector<vert_pair> edge_2_endpt;
        std::vector<CCORE> sup;
        std::vector<CCORE> trussness;
        std::vector<CCORE> node_trussness;
        CEDGEID edge_num;
        
        explicit truss_decomposition(std::vector<std::vector<CNODEID>> &ori_adj, int thread_num = 1) : porigin_adj(&ori_adj) {
            adj_t &origin_adj = *porigin_adj;
            for (auto i = 0; i < origin_adj.size(); ++i) {
                std::sort(origin_adj[i].begin(), origin_adj[i].end());
            }
            
            edge_num = 0;
            for (auto i = 0; i < ori_adj.size(); ++i) {
                std::vector<CNODEID> cur_track;
                for (auto j: ori_adj[i]) {
                    if (j > i) {
                        cur_track.emplace_back(j);
                        edge_map.emplace(vert_pair(i, j), edge_num);
                        edge_2_endpt.emplace_back(i, j);
                        ++edge_num;
                    }
                }
                cur_adj.emplace_back(cur_track);
            }
            sup.resize(edge_num, 0);
            
            support_init(thread_num);
        }
        
        void support_init(int thread_num) {
            if (thread_num == 1){
                for (auto i = 0; i < edge_num; ++i) {
                    CNODEID ep1 = edge_2_endpt[i].first;
                    CNODEID ep2 = edge_2_endpt[i].second;
                    std::vector<CNODEID> tmp_intersection;
                    tmp_intersection.reserve(32);
                    std::set_intersection(cur_adj[ep1].begin(), cur_adj[ep1].end(), cur_adj[ep2].begin(),
                                          cur_adj[ep2].end(), std::insert_iterator<std::vector<CNODEID >>(tmp_intersection,
                                                                                                          tmp_intersection.begin()));
                    for (auto ep3: tmp_intersection) {
                        CEDGEID edge1 = edge_map[std::make_pair(ep1, ep3)];
                        CEDGEID edge2 = edge_map[std::make_pair(ep2, ep3)];
                        sup[i]++;
                        sup[edge1]++;
                        sup[edge2]++;
                    }
                }
                return;
            }
            else {

#pragma omp parallel for schedule(dynamic) num_threads(thread_num)
                for (auto i = 0; i < edge_num; ++i) {
                    CNODEID ep1 = edge_2_endpt[i].first;
                    CNODEID ep2 = edge_2_endpt[i].second;
                    std::vector<CNODEID> tmp_intersection;
                    tmp_intersection.reserve(32);
                    std::set_intersection(cur_adj[ep1].begin(), cur_adj[ep1].end(), cur_adj[ep2].begin(),
                                          cur_adj[ep2].end(),
                                          std::insert_iterator<std::vector<CNODEID >>(tmp_intersection,
                                                                                      tmp_intersection.begin()));
                    for (auto ep3: tmp_intersection) {
//                    CEDGEID edge1 = edge_map.find(vert_pair(ep1, ep3))->second;
                        CEDGEID edge1 = edge_map[std::make_pair(ep1, ep3)];
                        CEDGEID edge2 = edge_map[std::make_pair(ep2, ep3)];
//                    CEDGEID edge2 = edge_map.find(vert_pair(ep2, ep3))->second;
#pragma omp atomic
                        sup[i]++;
#pragma omp atomic
                        sup[edge1]++;
#pragma omp atomic
                        sup[edge2]++;
                    }
                }
            }
        }
        
        void Parallel_K_Truss_Decomp(uint32_t thrNum) {
            adj_t &origin_adj = *porigin_adj;
            
            omp_set_num_threads(thrNum);
            
            std::cout << "truss decomp begin!\n";
            std::vector<CCORE> h(sup);
            std::vector<bool> scheduled(edge_num, true);
            bool updated = true;
            while (updated) {
                updated = false;

#pragma omp parallel for schedule(dynamic) num_threads(thrNum)
                for (auto e = 0; e < edge_num; ++e) {
                    if (!scheduled[e])
                        continue;
                    
                    std::vector<CNODEID> L;
                    std::vector<CNODEID> N;
                    
                    CNODEID ep1 = edge_2_endpt[e].first;
                    CNODEID ep2 = edge_2_endpt[e].second;
                    std::vector<CNODEID> tmp_intersection;
                    std::set_intersection(origin_adj[ep1].begin(), origin_adj[ep1].end(), origin_adj[ep2].begin(),
                                          origin_adj[ep2].end(),
                                          std::insert_iterator<std::vector<CNODEID >>(tmp_intersection,
                                                                                      tmp_intersection.begin()));
                    for (auto ep3: tmp_intersection) {
                        CEDGEID edge1 = Find_Edge(ep1, ep3);
                        CEDGEID edge2 = Find_Edge(ep2, ep3);
                        N.emplace_back(edge1);
                        N.emplace_back(edge2);
                        CUINT ru = std::min(h[edge1], h[edge2]);
                        L.emplace_back(ru);
                    }
                    
                    CUINT H = H_Index(L);
                    if (h[e] != H) {
#pragma omp critical
                        {
                            updated = true;
                        }
                        for (auto en: N) {
                            if (H < h[en] && h[en] <= h[e]) {
#pragma omp critical
                                {
                                    scheduled[en] = true;
                                }
                            }
                        }
                    }
#pragma omp critical
                    {
                        h[e] = H;
                        scheduled[e] = false;
                    }
                    
                }
            }
            trussness.resize(edge_num, 0);
            for (auto e = 0; e < edge_num; ++e) {
                trussness[e] = h[e] + 2;
            }
            std::cout << "truss decomp done!\n";
            
            //  赋予每个顶点一个trussness
            node_trussness.resize(cur_adj.size(), 2);
            for (auto i = 0; i < edge_num; ++i) {
                CNODEID ep1 = edge_2_endpt[i].first;
                CNODEID ep2 = edge_2_endpt[i].second;
                CCORE cur_trussness = trussness[i];
                node_trussness[ep1] = std::max(static_cast<CCORE >(node_trussness[ep1]), cur_trussness);
                node_trussness[ep2] = std::max(static_cast<CCORE >(node_trussness[ep2]), cur_trussness);
            }
        }
        inline int Del_Edge(CCORE trussness, CEDGEID del_edge, std::queue<CEDGEID>& need_del_queue){
            CCORE min_sup = trussness - 2;
            sup[del_edge] = 0;
            auto node1 = edge_2_endpt[del_edge].first;
            auto node2 = edge_2_endpt[del_edge].second;
            std::vector<CNODEID>::iterator bi1 = cur_adj[node1].begin();
            std::vector<CNODEID>::iterator ei1 = cur_adj[node1].end();
            std::vector<CNODEID>::iterator bi2 = cur_adj[node2].begin();
            std::vector<CNODEID>::iterator ei2 = cur_adj[node2].end();
            while (bi1 != ei1 && bi2 != ei2){
                if (*bi1 > *bi2) bi2++;
                else if (*bi2 > *bi1) bi1++;
                else{
                    CEDGEID edge1 = edge_map[std::make_pair(node1, *bi1)];
                    CEDGEID edge2 = edge_map[std::make_pair(node2, *bi2)];
                    if (sup[edge1] == min_sup){
                        need_del_queue.emplace(edge1);
                        sup[edge1] = 0;
                    } else sup[edge1]--;
                    if (sup[edge2] == min_sup){
                        need_del_queue.emplace(edge1);
                        sup[edge2] = 0;
                    } else  sup[edge2]--;
                    bi1++;
                    bi2++;
                }
            }
            return 0;
        }
        std::set<CNODEID> Judge_K_Truss(CNODEID need_test, CEDGEID trussness){
            std::queue<CEDGEID> need_del;
            
            CCORE min_sup = trussness -2;
            for (auto edge = 0; edge< sup.size(); ++edge){
                if (sup[edge] < min_sup){
                    need_del.emplace(edge);
                }
            }
            while (!need_del.empty()){
                Del_Edge(trussness, need_del.front(), need_del);
                need_del.pop();
            }
            std::set<CNODEID> res;
            for (auto e = 0; e < sup.size(); ++e){
                if (sup[e] >= min_sup){
                    res.emplace(edge_2_endpt[e].first);
                    res.emplace(edge_2_endpt[e].second);
                }
            }
            if (res.find(need_test) == res.end()){
                res.clear();
            }
            
            return res;
        }
        
        CEDGEID Find_Edge(CNODEID ep1, CNODEID ep2) {
            CEDGEID res = -1;
            auto pres = edge_map.find(std::make_pair(std::min(ep1, ep2), std::max(ep1, ep2)));
            if (pres != edge_map.end()){
                res = pres->second;
            }
            return res;
        }
        
        inline CUINT H_Index(std::vector<CNODEID> &L) {
            
            int32_t size = L.size();
            CUINT resH = 0;
            if (size == 0) {
                return 0;
            }
            std::vector<CNODEID> tmp_vec(L.size() + 1, 0);
            for (auto i: L) {
                if (i >= size)
                    tmp_vec[size]++;
                else
                    tmp_vec[i]++;
            }
            
            CUINT cnt = 0;
            for (auto i = size; i >= 0; --i) {
                cnt += tmp_vec[i];
                
                if (cnt >= i) {
                    resH = i;
                    break;
                }
            }
            
            return resH;
        }
    };

}


#endif //CNNS_DECOMPOSITION_H
