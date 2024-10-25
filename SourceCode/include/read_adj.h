// The code in this file is used to read dataset information.
#ifndef CNNS1_READ_ADJ_H
#define CNNS1_READ_ADJ_H

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <queue>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <cassert>
#include <bitset>
#include <initializer_list>
#include <algorithm>
#include <mutex>
#include <phmap.h>
#include "basic_define.h"
#include "basic_data.h"

using phmap::flat_hash_map;
using phmap::parallel_flat_hash_map;
using phmap::flat_hash_set;
using phmap::parallel_flat_hash_set;

namespace chci{

    class read_adj {
    public:
        using VERT_T = CNODEID;
        using EDGE_T = CEDGEID;

        read_adj(const std::string &graph_path, const std::string &vertex_path, Dataset_Type speci_dataset,
                 const std::string &edge_path = "") {
            std::cout.setstate(std::ios::failbit);
            switch (speci_dataset) {
                case small_dblp:
                Paper_No = 1;
                Author_No = 0;
                    read_adj_loose_DBLP(graph_path, vertex_path, small_dblp);
                    break;
                case DBLP:
                read_adj_loose_DBLP(graph_path, vertex_path, DBLP);
                    break;
                case IMDB_Person:
                std::cout << edge_path << std::endl;
                    read_adj_heter(graph_path, vertex_path, edge_path, speci_dataset);
                    break;
                case IMDB_Movie:
                std::cout << edge_path << std::endl;
                    read_adj_heter(graph_path, vertex_path, edge_path, speci_dataset);
                break;
                case FourSquare:
                    std::cout << edge_path << std::endl;
                    read_adj_heter(graph_path, vertex_path, edge_path, speci_dataset);
                    break;
                case CORA:
                    read_adj_homo(edge_path);
                    break;
                case Twitch:
                    read_adj_homo(edge_path);
                    break;
                case LiveJournal:
                    read_adj_homo(edge_path);
                    break;
                case Facebook:
                    read_adj_homo(edge_path);
                    break;
                case FB_MERGE_ORI:
                    read_adj_homo(edge_path);
                    break;
                case FB_MERGE:
                    read_adj_homo(edge_path);
                    break;
                default:
                    read_adj_homo(edge_path);
                    break;
            }
            for (auto &i : cur_2_cur_adj){
                std::sort(i.begin(), i.end());
            }
            std::cout.clear();
       }

        CSTATUS read_adj_loose_DBLP(const std::string &graph_path, const std::string &vertex_path, Dataset_Type speci_dataset)
        {
            if (speci_dataset == small_dblp) {
                Paper_No = 1;
                Author_No = 0;
            } else if (speci_dataset == DBLP) {
                Paper_No = 0;
                Author_No = 1;
            }
            assert(Paper_No + 1 != 0 && Author_No + 1 != 0);
            std::vector<std::vector<CNODEID>> ori_graph;
            std::vector<VERT_T> ori_vertex;
            std::vector<CNODEID> tmp_2_ori;
            std::unordered_map<CNODEID, CNODEID> ori_2_tmp_map;
            std::ifstream graph_strm(graph_path);

            std::vector<CNODEID> graph_vec;
            std::string str;

            CNODEID count = 0;

            std::vector<int> valid_vert(total_node_num, -1);
            std::ifstream vertex_strm(vertex_path);
            if (!vertex_strm) std::cout << "Open VERTEX FILE Failed!!!\n";
            else {
                while (!vertex_strm.eof()) {

                    std::getline(vertex_strm, str);
                    if (str == "") {
                        continue;
                    }

                    if ((count++) % 10000 == 0) {
                        std::cout << "Vertex: executed " << count - 1 << "lines" << std::endl;
                    }
                    std::istringstream tmp_in(str);

                    CNODEID tmp_vert;
                    tmp_in >> tmp_vert;
                    tmp_2_ori.emplace_back(tmp_vert);
                    ori_2_tmp_map.emplace(tmp_vert, count - 1);
                    CNODEID tmp_type;
                    tmp_in >> tmp_type;
                    ori_vertex.emplace_back(tmp_type);
                }
            }

            count = 0;
            vertex_strm.close();

            if (!graph_strm) std::cout << "Open GRAPH DATA Failed!!\n";
            else {
                while (!graph_strm.eof()) {
                    if ((count++) % 10000 == 0) {
                        std::cout << "Graph: executed  " << count - 1 << "lines" << std::endl;
                    }
                    std::getline(graph_strm, str);
                    if (str == "")
                        continue;
                    std::istringstream tmp_in(str);

                    CNODEID tmp;
                    std::vector<CNODEID> tmp_vec;
                    int i = 1;
                    tmp_in >> tmp;
                    if (ori_2_tmp_map.find(tmp) == ori_2_tmp_map.end())
                        continue;
                    while (tmp_in >> tmp) {
                        if (i++ % 2) {
                            if (ori_2_tmp_map.find(tmp) == ori_2_tmp_map.end())
                                continue;
                            tmp_vec.emplace_back(tmp);
                        }
                    }
                    ori_graph.emplace_back(tmp_vec);
                }
            }
            count = 0;
            graph_strm.close();



            CNODEID cur_key = 0;
            for (CNODEID i = 0; i < ori_graph.size(); i++) {
                if ((count++) % 10000 == 0) {
                    std::cout << "Find Neighbors: executed " << count - 1 << "lines" << std::endl;
                }
                if (ori_vertex[i] == Author_No) {
                    ori_2_cnns.emplace(tmp_2_ori[i], cur_key++);
                    cnns_2_ori.emplace_back(tmp_2_ori[i]);
                    std::vector<CNODEID> tmp_vec;
                    std::set<CNODEID> tmp_set;
                    for (auto j: ori_graph[i]) {
                        CNODEID cur_j_tmp_id = ori_2_tmp_map.find(j)->second;
                        if (ori_vertex[cur_j_tmp_id] == Paper_No) {
                            for (auto k: ori_graph[cur_j_tmp_id]) {
                                CNODEID cur_k_tmp_id = ori_2_tmp_map.find(k)->second;
                                if ((cur_k_tmp_id != i) && ori_vertex[cur_k_tmp_id] == Author_No) {
                                    tmp_set.insert(k);
                                }
                            }
                        }
                    }
                    for (auto m: tmp_set) {
                        tmp_vec.emplace_back(m);
                    }
                    res_graph.emplace_back(tmp_vec);
                }
            }

            total_node_num = res_graph.size();
            AUTHOR_NUM = res_graph.size();
            cur_2_cur_adj = std::vector<std::vector<VERT_T>>(AUTHOR_NUM);

            for (auto i = 0; i < res_graph.size(); i++) {
                std::vector<VERT_T> cur_graph_neigh_id_set;
                for (auto j = 0; j < res_graph[i].size(); j++) {
                    auto tmp_cnns_id = ori_2_cnns.find(res_graph[i][j])->second;
                    cur_graph_neigh_id_set.emplace_back(tmp_cnns_id);
                }
                cur_2_cur_adj[i] = cur_graph_neigh_id_set;
                cur_graph_neigh_id_set.clear();
            }
            return 0;
        }

        void read_adj_homo(const std::string &edge_path){

            std::vector<std::vector<std::pair<CNODEID, CEDGEID>>> ori_graph;
            std::vector<std::vector<std::pair<CNODEID, EDGE_T>>> ori_graph_with_edget;
            std::vector<VERT_T> ori_vertex;
            std::vector<CNODEID> tmp_2_ori;
            std::unordered_map<CNODEID, CNODEID> ori_2_tmp_map;
            size_t edge_num;
            std::vector<CNODEID> graph_vec;
            std::string str;

            CNODEID count = 0;
            std::ifstream edge_strm(edge_path);
            if (!edge_strm) std::cout << "Open VERTEX FILE Failed!!!\n";
            else {
                while (!edge_strm.eof()) {

                    std::getline(edge_strm, str);
                    if (str == "") {
                        continue;
                    }

                    std::istringstream tmp_in(str);

                    CNODEID cur_id;
                    tmp_in >> cur_id;
                    if (ori_2_cnns.find(cur_id) == ori_2_cnns.end()){
                        cnns_2_ori.emplace_back(cur_id);
                        ori_2_cnns.emplace(cur_id, count);
                        count++;
                    }
                    CNODEID target_id;
                    tmp_in >> target_id;
                    if (ori_2_cnns.find(target_id) == ori_2_cnns.end()){
                        cnns_2_ori.emplace_back(target_id);
                        ori_2_cnns.emplace(target_id, count);
                        count++;
                    }
                }
            }
            edge_strm.close();

            res_graph = std::vector<std::vector<CNODEID>>(count);

            std::ifstream new_edge_strm(edge_path);
            if (!new_edge_strm) std::cout << "Open VERTEX FILE Failed!!!\n";
            else {
                while (!new_edge_strm.eof()) {

                    std::getline(new_edge_strm, str);
                    if (str == "") {
                        continue;
                    }

                    std::istringstream tmp_in(str);

                    CNODEID cur_id;
                    tmp_in >> cur_id;

                    CNODEID target_id;
                    tmp_in >> target_id;

                    res_graph[ori_2_cnns[cur_id]].emplace_back(target_id);
                }
            }
            new_edge_strm.close();
            total_node_num = count;
            AUTHOR_NUM = res_graph.size();
            cur_2_cur_adj = std::vector<std::vector<VERT_T>>(AUTHOR_NUM);

            for (auto i = 0; i < res_graph.size(); i++) {
                std::vector<VERT_T> cur_graph_neigh_id_set;
                for (auto j = 0; j < res_graph[i].size(); j++) {
                    auto tmppt =ori_2_cnns.find(res_graph[i][j]) ;
                    if (tmppt != ori_2_cnns.end()){
                        auto tmp_cnns_id = tmppt->second;
                        cur_graph_neigh_id_set.emplace_back(tmp_cnns_id);
                    }
                }
                cur_2_cur_adj[i] = cur_graph_neigh_id_set;
                cur_graph_neigh_id_set.clear();
            }

            for (auto i = 0; i < total_node_num; i++){
                for (auto j :cur_2_cur_adj[i]){
                    if (std::find(cur_2_cur_adj[j].begin(), cur_2_cur_adj[j].end(), i) == cur_2_cur_adj[j].end()){
                        cur_2_cur_adj[j].emplace_back(i);
                    }
                }
            }

            for (auto i = 0; i < total_node_num; ++i){
                std::vector<CNODEID> cur_ori_neigh_l;
                for (auto j : cur_2_cur_adj[i]){
                    cur_ori_neigh_l.emplace_back(cnns_2_ori[j]);
                }
                res_graph[i] = cur_ori_neigh_l;
            }

        }
        void read_adj_heter(const std::string &graph_path, const std::string &vertex_path, const std::string &edge_path, Dataset_Type datasetType_in) {

            std::vector<std::vector<std::pair<CNODEID, CEDGEID>>> ori_graph;
            std::vector<std::vector<std::pair<CNODEID, EDGE_T>>> ori_graph_with_edget;
            std::vector<VERT_T> ori_vertex;
            std::vector<CNODEID> tmp_2_ori;
            std::unordered_map<CNODEID, CNODEID> ori_2_tmp_map;
            size_t edge_num;
            std::vector<CNODEID> graph_vec;
            std::string str;

            CNODEID count = 0;

            std::ifstream vertex_strm(vertex_path);
            if (!vertex_strm) std::cout << "Open VERTEX FILE Failed!!!\n";
            else {
                while (!vertex_strm.eof()) {

                    std::getline(vertex_strm, str);
                    if (str == "") {
                        continue;
                    }

                    if ((count++) % 100000 == 0) {
                        std::cout << "Vertex: executed " << count - 1 << "lines" << std::endl;
                    }
                    std::istringstream tmp_in(str);

                    CNODEID tmp_vert;
                    tmp_in >> tmp_vert;
                    tmp_2_ori.emplace_back(tmp_vert);
                    ori_2_tmp_map.emplace(tmp_vert, count - 1);
                    CNODEID tmp_type;
                    tmp_in >> tmp_type;
                    ori_vertex.emplace_back(tmp_type);
                }
            }

            total_node_num = count;
            count = 0;
            vertex_strm.close();

            std::ifstream graph_strm(graph_path);
            if (!graph_strm) std::cout << "Open GRAPH DATA Failed!!\n";
            else {
                while (!graph_strm.eof()) {
                    if ((count++) % 100000 == 0) {
                        std::cout << "Graph: executed  " << count - 1 << "lines" << std::endl;
                    }
                    std::getline(graph_strm, str);
                    if (str == "")
                        continue;
                    std::istringstream tmp_in(str);

                    CNODEID ori_id;
                    std::vector<std::pair<CNODEID, CEDGEID>> tmp_vec;
                    tmp_in >> ori_id;
                    if (ori_2_tmp_map.find(ori_id) == ori_2_tmp_map.end())
                        continue;
                    CNODEID tmp;
                    while (tmp_in >> tmp) {
                        CNODEID cur_neigh_id = tmp;
                        CEDGEID cur_edge_id = -1;
                        tmp_in >> cur_edge_id;
                        edge_num++;
                        tmp_vec.emplace_back(cur_neigh_id, cur_edge_id);
                    }
                    ori_graph.emplace_back(tmp_vec);
                }
            }
            count = 0;
            graph_strm.close();


            std::vector<EDGE_T> edge_attr_vec;
            std::ifstream edge_strm(edge_path);
            if (!edge_strm) std::cout << "Open Edge DATA Failed!!\n";
            else {
                while (!edge_strm.eof()) {
                    if ((count++) % 10000 == 0) {
                        std::cout << "Edge: executed  " << count - 1 << "lines" << std::endl;
                    }
                    std::getline(edge_strm, str);
                    if (str == "")
                        continue;
                    std::istringstream tmp_in(str);

                    CNODEID cur_edge_id;
                    tmp_in >> cur_edge_id;
                    EDGE_T cur_edge_attr = -1;
                    tmp_in >> cur_edge_attr;

                    assert(cur_edge_id == edge_attr_vec.size());
                    edge_attr_vec.emplace_back(cur_edge_attr);
                }
            }
            count = 0;
            edge_strm.close();



            for (auto i: ori_graph) {
                std::vector<std::pair<CNODEID, EDGE_T>> tmp_vec;
                for (auto j: i) {
                    tmp_vec.emplace_back(j.first, edge_attr_vec[j.second]);
                }
                ori_graph_with_edget.emplace_back(tmp_vec);
            }

            VERT_T n1 = -1;
            EDGE_T e2 = -1;
            VERT_T n2 = -1;
            EDGE_T e3 = -1;
            VERT_T n3 = -1;
            EDGE_T e4 = -1;
            VERT_T n4 = -1;
            EDGE_T e5 = -1;
            VERT_T n5 = -1;
            switch (datasetType_in) {
                case DBpedia_Basketballplayer:
                      n1 = 39;
                      e2 = 169;
                      n2 = 112;
                      e3 = 1925;
                      n3 = 39;
                    Deal_MetaPath_MultiThread(ori_graph_with_edget, ori_vertex, ori_2_tmp_map, tmp_2_ori, n1, e2, n2, e3, n3);
                    break;
                case DBpedia_Soccerplayer:
                    n1 = 70;
                    e2 = 325;
                    n2 = 67;
                    e3 = 310;
                    n3 = 70;
                    

                    Deal_MetaPath_MultiThread(ori_graph_with_edget, ori_vertex, ori_2_tmp_map, tmp_2_ori, n1, e2, n2, e3, n3);
                    break;
                case DBpedia_Automobile:

                    n1 = 159;
                    e2 = 1677;
                    n2 = 41;
                    e3 = 3153;
                    n3 = 159;
                    Deal_MetaPath_MultiThread(ori_graph_with_edget, ori_vertex, ori_2_tmp_map, tmp_2_ori, n1, e2, n2, e3, n3);
                    break;
                case FourSquare:
                      n1 = 0;
                      e2 = 0;
                      n2 = 3;
                      e3 = 6;
                      n3 = 0;
                    Deal_MetaPath_MultiThread(ori_graph_with_edget, ori_vertex, ori_2_tmp_map, tmp_2_ori, n1, e2, n2, e3, n3);
                    break;
                case IMDB_Movie:
                     n1 = 0;
                     e2 = 3;
                     n2 = 1;
                     e3 = 12;
                     n3 = 0;
                     e4 = 0;
                     n4 = 1;
                     e5 = 15;
                     n5 = 0;
                    Deal_MetaPath_MultiThread(ori_graph_with_edget, ori_vertex, ori_2_tmp_map, tmp_2_ori, n1, e2, n2, e3, n3, e4, n4, e5, n5);
                    break;
                case IMDB_Person:
                     n1 = 1;
                     e2 = 21;
                     n2 = 0;
                     e3 = 9;
                     n3 = 1;
                    Deal_MetaPath_MultiThread(ori_graph_with_edget, ori_vertex, ori_2_tmp_map, tmp_2_ori, n1, e2, n2, e3, n3);
                    break;
            }

            AUTHOR_NUM = res_graph.size();
            total_node_num = AUTHOR_NUM;
            cur_2_cur_adj = std::vector<std::vector<VERT_T>>(AUTHOR_NUM);

            for (auto i = 0; i < res_graph.size(); i++) {
                std::vector<VERT_T> cur_graph_neigh_id_set;
                for (auto j = 0; j < res_graph[i].size(); j++) {
                    auto tmppt =ori_2_cnns.find(res_graph[i][j]) ;
                    if (tmppt != ori_2_cnns.end()){
                        auto tmp_cnns_id = ori_2_cnns.find(res_graph[i][j])->second;
                        cur_graph_neigh_id_set.emplace_back(tmp_cnns_id);
                    }
                }
                cur_2_cur_adj[i] = cur_graph_neigh_id_set;
                cur_graph_neigh_id_set.clear();
            }

        }
        void Deal_MetaPath_MultiThread( std::vector<std::vector<std::pair<CNODEID, EDGE_T>>> &ori_graph_with_edget,
                                        std::vector<CNODEID> &ori_vertex,
                                        std::unordered_map<CNODEID , CNODEID > ori_2_tmp_map,
                                        std::vector<CNODEID> tmp_2_ori,
                                        VERT_T n1, EDGE_T e2, VERT_T n2, EDGE_T e3, VERT_T n3){
            CNODEID count = 0;
            CNODEID cur_key = 0;
            
            
            std::mutex write_add_lock;
            std::vector<CNODEID> cur_type_id_record;
            std::unordered_map<CNODEID, CNODEID> tmp_2_cur_type_id;
            CNODEID cnt = 0;
            for (CNODEID fn1 = 0; fn1 < ori_vertex.size(); fn1++){
                if (ori_vertex[fn1] == n1){
                    cur_type_id_record.emplace_back(tmp_2_ori[fn1]);
                    tmp_2_cur_type_id.emplace( fn1, cnt++);
                }
            }
            
            std::vector<std::vector<CNODEID>*> all_cur_id_neighbor_adj(cnt, nullptr);
            
            CNODEID cnt_valid = 0;
#pragma omp parallel for schedule(dynamic) num_threads(16)
            for (CNODEID i = 0; i < ori_graph_with_edget.size(); i++) {
                if ((count++) % 10000 == 0) {
                    std::cout << "Find Neighbors: executed " << count - 1 << "lines" << std::endl;
                }
                if (ori_vertex[i] == n1) {
                    std::set<CNODEID> tmp_set;
                    for (auto j: ori_graph_with_edget[i]) {
                        
                        auto tmpptr = ori_2_tmp_map.find(j.first);
                        if (tmpptr == ori_2_tmp_map.end()){
                            continue;
                        }
                        CNODEID cur_j_tmp_id = tmpptr->second;
                        VERT_T cur_j_edge_id = j.second;
                        if (ori_vertex[cur_j_tmp_id] == n2 && cur_j_edge_id == e2) {
                            
                            for (auto k: ori_graph_with_edget[cur_j_tmp_id]) {

                                tmpptr = ori_2_tmp_map.find(k.first);
                                if (tmpptr == ori_2_tmp_map.end()){
                                    continue;
                                }
                                CNODEID cur_k_tmp_id = ori_2_tmp_map.find(k.first)->second;
                                VERT_T cur_k_edge_t = k.second;
                                if ((cur_k_tmp_id != i) && ori_vertex[cur_k_tmp_id] == n3 && cur_k_edge_t == e3) {
                                    tmp_set.insert(k.first);
                                }
                            }
                        }
                    }
                    
                    CNODEID tmp_set_sz = tmp_set.size();
                    if (tmp_set_sz == 0){
                        continue;
                    }
                    
                    std::vector<CNODEID>* tmp_vec = new std::vector<CNODEID>(tmp_set.begin(), tmp_set.end());

                        std::unique_lock<std::mutex> lkw(write_add_lock);
                        all_cur_id_neighbor_adj[tmp_2_cur_type_id.find(i)->second] = tmp_vec;
#pragma omp critical
                    {
                        cnt_valid++;
                    }
                }
            }
            
            cnns_2_ori.resize(cnt_valid, -1);
            ori_2_cnns.reserve(cnt_valid);
            CNODEID cnns_cnt = 0;
            for (CNODEID i = 0; i < cnt; ++i){
                if (all_cur_id_neighbor_adj[i] != nullptr){
                    res_graph.emplace_back(*(all_cur_id_neighbor_adj[i]));
                    ori_2_cnns.emplace(cur_type_id_record[i], cnns_cnt);
                    cnns_2_ori[cnns_cnt] = cur_type_id_record[i];
                    cnns_cnt++;
                }
            }
            for (auto i : all_cur_id_neighbor_adj){
                if (i != nullptr)
                    delete i ;
            }
        }
        
        void Deal_MetaPath_MultiThread( std::vector<std::vector<std::pair<CNODEID, EDGE_T>>> &ori_graph_with_edget,
                            std::vector<CNODEID> &ori_vertex,
                            std::unordered_map<CNODEID , CNODEID > ori_2_tmp_map,
                            std::vector<CNODEID> tmp_2_ori,
                            VERT_T n1, EDGE_T e2, VERT_T n2, EDGE_T e3, VERT_T n3, EDGE_T e4 , VERT_T n4, EDGE_T e5, VERT_T n5){
            CNODEID count = 0;
            CNODEID cur_key = 0;
            
            
            std::mutex write_add_lock;
            std::vector<CNODEID> cur_type_id_record;
            std::unordered_map<CNODEID, CNODEID> tmp_2_cur_type_id;
            CNODEID cnt = 0;
            for (CNODEID fn1 = 0; fn1 < ori_vertex.size(); fn1++){
                if (ori_vertex[fn1] == n1){
                    cur_type_id_record.emplace_back(tmp_2_ori[fn1]);
                    tmp_2_cur_type_id.emplace( fn1, cnt++);
                }
            }
            
            std::vector<std::vector<CNODEID>*> all_cur_id_neighbor_adj(cnt, nullptr);
            
            CNODEID cnt_valid = 0;
            
#pragma omp parallel for schedule(dynamic) num_threads(16)
            for (CNODEID fn1 = 0; fn1 < ori_graph_with_edget.size(); fn1++) {
                if (ori_vertex[fn1] == n1) {
                    
                    std::set<CNODEID> tmp_set;
                    for (auto fn2: ori_graph_with_edget[fn1]) {
                        

                        auto tmpptr = ori_2_tmp_map.find(fn2.first);
                        if (tmpptr == ori_2_tmp_map.end()){
                            continue;
                        }
                        CNODEID cur_fn2_tmp_id = tmpptr->second;
                        VERT_T cur_j_edge_id = fn2.second;
                        if (ori_vertex[cur_fn2_tmp_id] == n2 && cur_j_edge_id == e2) {
                            for (auto fn3: ori_graph_with_edget[cur_fn2_tmp_id]) {
                                
                                tmpptr = ori_2_tmp_map.find(fn3.first);
                                if (tmpptr == ori_2_tmp_map.end()){
                                    continue;
                                }
                                CNODEID cur_fn3_tmp_id = tmpptr->second;
                                VERT_T cur_fn3_edge_t = fn3.second;
                                if ((cur_fn3_tmp_id != fn1) && ori_vertex[cur_fn3_tmp_id] == n3 && cur_fn3_edge_t == e3) {
                                    for (auto fn4: ori_graph_with_edget[cur_fn3_tmp_id]) {
                                        tmpptr = ori_2_tmp_map.find(fn4.first);
                                        if (tmpptr == ori_2_tmp_map.end()) {
                                            continue;
                                        }
                                        CNODEID cur_fn4_tmp_id = tmpptr->second;
                                        VERT_T cur_fn4_edge_t = fn4.second;
                                        if ((cur_fn4_tmp_id != fn2.first) && ori_vertex[cur_fn4_tmp_id] == n4 && cur_fn4_edge_t == e4) {
                                            for (auto fn5: ori_graph_with_edget[cur_fn4_tmp_id]) {
                                                tmpptr = ori_2_tmp_map.find(fn5.first);
                                                if (tmpptr == ori_2_tmp_map.end()) {
                                                    continue;
                                                }
                                                CNODEID cur_fn5_tmp_id = tmpptr->second;
                                                VERT_T cur_fn5_edge_t = fn5.second;
                                                if ((cur_fn5_tmp_id != fn3.first) && ori_vertex[cur_fn5_tmp_id] == n5 && cur_fn5_edge_t == e5 && cur_fn5_tmp_id != fn1) {
                                                    tmp_set.insert(fn5.first);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    
                    CNODEID tmp_set_sz = tmp_set.size();
                    if (tmp_set_sz == 0){
                        continue;
                    }
                    
                    std::vector<CNODEID>* tmp_vec = new std::vector<CNODEID>(tmp_set.begin(), tmp_set.end());
                    
                    std::unique_lock<std::mutex> lkw(write_add_lock);
                    all_cur_id_neighbor_adj[tmp_2_cur_type_id.find(fn1)->second] = tmp_vec;
#pragma omp critical
                    {
                        cnt_valid++;
                    }

                }
            }
            
            cnns_2_ori.resize(cnt_valid, -1);
            ori_2_cnns.reserve(cnt_valid);
            CNODEID cnns_cnt = 0;
            for (CNODEID i = 0; i < cnt; ++i){
                if (all_cur_id_neighbor_adj[i] != nullptr){
                    res_graph.emplace_back(*(all_cur_id_neighbor_adj[i]));
                    ori_2_cnns.emplace(cur_type_id_record[i], cnns_cnt);
                    cnns_2_ori[cnns_cnt] = cur_type_id_record[i];
                    cnns_cnt++;
                }
            }
            for (auto i : all_cur_id_neighbor_adj){
                if (i != nullptr)
                    delete i ;
            }
        }
        
        VERT_T Paper_No = -1;
        VERT_T Author_No = -1;

        CNODEID total_node_num = 0;
        CNODEID AUTHOR_NUM = 0;
        std::vector<std::vector<CNODEID>> res_graph;
        parallel_flat_hash_map<CNODEID, CNODEID> ori_2_cnns;
        std::vector<CNODEID> cnns_2_ori;
        std::vector<std::vector<CNODEID>> cur_2_cur_adj;


    };
}
#endif //CNNS1_READ_ADJ_H
