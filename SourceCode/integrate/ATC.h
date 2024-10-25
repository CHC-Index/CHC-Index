#ifndef READ_ADJ_H_ATC_H
#define READ_ADJ_H_ATC_H

#include <iostream>
#include "../include/chci.h"
#include "../include/basic_define.h"
#include <ctime>

namespace chci {

    //  指定数据类型与acqdata长度
    template<int numeric_num, int text_num>
    class atc : public chci_graph<numeric_num, text_num> {
//        int num = numeric_num + text_num;
        using chci_graph<numeric_num, text_num>::core_list;
        using chci_graph<numeric_num, text_num>::cnns_2_ori;
        using chci_graph<numeric_num, text_num>::node_num_per_compo;
        using chci_graph<numeric_num, text_num>::Get_Current_Component_By_Cnns_Id;
        using chci_graph<numeric_num, text_num>::Load_Graph_bin;
        using chci_graph<numeric_num, text_num>::Load_Graph_bin_part;
        using chci_graph<numeric_num, text_num>::ori_2_cnns_map;
        using chci_graph<numeric_num, text_num>::cur_graph_ori_adj;
        using chci_graph<numeric_num, text_num>::compo_l;
        using chci_graph<numeric_num, text_num>::ntotal_;
        using chci_graph<numeric_num, text_num>::data_;
        std::vector<std::bitset<numeric_num+text_num>> atc_data_;
        int search_threads = 1;

    public:
        atc() = default;

        atc(const CCHAR *graph_path, const CCHAR *vertex_path, const CCHAR *data_path, const CCHAR *edge_path,
            Dataset_Type speci_dataset, StructureMethod sMethod, Cmodel com_base, float alpha, float B,
            CUINT buildQueueLength, CDEGREE maxDegree, CUINT threadNum)
                : chci_graph<numeric_num, text_num>(graph_path, vertex_path, data_path, edge_path, speci_dataset, TEXT, sMethod, sMethod, com_base,
                                                    alpha, buildQueueLength, maxDegree, threadNum) {
            if (Init_ATC_Data() != 0){
                throw(std::runtime_error("err in atc data init"));
            }
        }

        void Load_Index(Dataset_Type ds_t, const std::string& cnns_index_input, const std::string& data_path, const std::string &graph_path,
                            const std::string &vertex_path, const std::string &edge_path) {
            Load_Graph_bin(ds_t, TEXT, truss, baseline, cnns_index_input, data_path, graph_path, vertex_path, edge_path);
            if (Init_ATC_Data() != 0){
                throw(std::runtime_error("err in atc data init"));
            }
        }
        void Load_Index(Dataset_Type ds_t, const std::string& cnns_index_input, const std::string& data_path, const std::string &graph_path,
                            const std::string &vertex_path, const std::string &edge_path, const std::string &valid_nodes_path) {
        //     if (valid_nodes_path != "null"){
        //     Load_Graph_bin_part(ds_t, TEXT, truss, baseline, cnns_index_input, data_path, graph_path, vertex_path, edge_path, valid_nodes_path);
        // } else {
            Load_Graph_bin(ds_t, TEXT, truss, baseline, cnns_index_input, data_path, graph_path, vertex_path, edge_path);
        // }
            if (Init_ATC_Data() != 0){
                throw(std::runtime_error("err in atc data init"));
            }
        }
        CSTATUS Init_ATC_Data(){
            if (data_.size() <= 0 || data_.size() != ntotal_){
                std::cerr << "Data is not loaded successfully!!!\n";
                return -100;
            }
            atc_data_.resize(ntotal_, 0);
            for (auto i = 0; i < ntotal_; ++i){
                for (auto j = 0; j < numeric_num; ++j){
                    if (data_[i].numeric_bin.test(j)){
                        atc_data_[i].set(j);
                    }
                }
                for (auto j = 0; j < text_num; ++j){
                    if (data_[i].text_bin.test(j)){
                        atc_data_[i].set(j+numeric_num);
                    }
                }
            }
            return 0;
        }

        CSTATUS Query_More_Nodes(std::vector<CNODEID> querySet, std::string res_path, CCORE coreness, int d,
                                 float gamma = 1.0) {

            // Loc_ATC might fail due to its special initialization strategy
            omp_set_num_threads(search_threads);
        std::cerr << "THREADS:" << search_threads << std::endl<< std::endl;

            std::mutex out_mutex;
            std::ofstream out(res_path);
            CDIST avr_Dist = 0;
            double avr_t = 0;
            int cnt = 0;

#pragma omp parallel for schedule(dynamic, 1)
            for (auto ii = 0; ii < querySet.size(); ++ii) {
                auto i = querySet[ii];
                std::vector<CNODEID> attr_cnt;
                attr_cnt.clear();
                attr_cnt.resize(numeric_num+text_num, 0);

                CDIST community_dist = 0;
                CNODEID query_CNNSID = ori_2_cnns_map.find(i)->second;
if (core_list[query_CNNSID] < coreness){
                    continue;
                }
                size_t time_us = 0;
                auto t1 = std::chrono::high_resolution_clock::now();
                int max_size = gamma * node_num_per_compo[coreness][compo_l[query_CNNSID][coreness]];
                auto tmpset = ATC_Query_Truss_Loc_Size_Bounded_kd(query_CNNSID, coreness, d, &community_dist, max_size,
                                                                  &time_us);
                auto t2 = std::chrono::high_resolution_clock::now();
                if (tmpset.size() < coreness)
                    continue;
                out_mutex.lock();
                cnt++;
                avr_t += time_us;
                avr_Dist += community_dist;
                out << std::to_string(i) << std::endl;
                for (auto j: tmpset) {
                    out << std::to_string(cnns_2_ori[j]) << " ";
                }
                out << std::endl;
                out << "Community Distance = " << community_dist << std::endl;
//                out << "Runtime : " << std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000>>>(t2-t1).count() << " ms";
                out << "Runtime : " << time_us << " mcrs";
                out << std::endl;

                std::cout << "w/oIndex Current query ID: " << std::to_string(i) << " Community Distance = "
                          << community_dist << " size = " << tmpset.size() << std::endl;
                std::cout << "Runtime : "
                          << std::chrono::duration_cast<std::chrono::microseconds>(
                                  t2 - t1).count() << " mcrs";
                std::cout << "LocATC Runtime : " << time_us << " mcrs";
                std::cout << std::endl;

                out_mutex.unlock();
            }

            avr_Dist /= cnt;
            avr_t /= cnt;
            out << "Avr dist = " << avr_Dist << std::endl;
            out << "Avr runtime = " << avr_t << std::endl;
//            out << "Fail Num = " << fail_num << std::endl;
            std::cout << "Avr dist = " << avr_Dist << std::endl;
            std::cout << "Avr runtime = " << avr_t << std::endl;
//            std::cout << "Fail Num = " << fail_num << std::endl;

            return 0;
        }

        CSTATUS
        Query_More_Nodes_withIndex_Formal_2cho(const std::string &CNNS_Res, const std::string &res_path,
                                               const std::string &wo_path, CCORE coreness,
                                               int d, int threads, int run_with, int run_wo,
                                               float gamma = 0) {

            search_threads = threads;
            omp_set_num_threads(search_threads);
            std::mutex out_mutex;

            std::ifstream in(CNNS_Res);
            if (!in.is_open()) {
                throw (std::runtime_error("Invalid Retrieve Nodes Path!!\n"));
                return 10000;
            }
            std::istringstream in_s;

            std::vector<std::pair<CNODEID, std::vector<CNODEID>>> query_with_cnnsRes;
            std::string tmpstr;
            while (!in.eof()) {
                std::getline(in, tmpstr);
                if (tmpstr == "") {
                    continue;
                }
                in_s.clear();
                in_s.str("");
                in_s.str(tmpstr);
                CNODEID tmp_id = 0;
                std::vector<CNODEID> tmp_res;
                in_s >> tmp_id;
                std::getline(in, tmpstr);
                in_s.clear();
                in_s.str("");
                in_s.str(tmpstr);
                CNODEID cur_res_nei = -1;
                while (in_s >> cur_res_nei) {
                    tmp_res.emplace_back(cur_res_nei);
                }
                query_with_cnnsRes.emplace_back(tmp_id, tmp_res);
                std::getline(in, tmpstr);
            }

            std::vector<CNODEID> queryS;
            queryS.reserve(query_with_cnnsRes.size());
            for (auto i: query_with_cnnsRes) {
                queryS.emplace_back(i.first);
            }

            CDIST avr_Dist = 0;
            double avr_t = 0;

            int cnt = 0;
            if (run_with > 0) {
                std::ofstream out(res_path);
#pragma omp parallel for schedule(dynamic)
                for (auto ii = 0; ii < query_with_cnnsRes.size(); ii++) {
                    auto &i_m = query_with_cnnsRes[ii];
                    CNODEID i = i_m.first;
                    CDIST community_dist = 0;
                    CNODEID query_CNNSID = ori_2_cnns_map.find(i_m.first)->second;
if (core_list[query_CNNSID] < coreness){
                    continue;
                }
                    size_t time_us = 0;

                    auto tmpset = ATC_Query_withIndex_Truss_Bulk(query_CNNSID, coreness, d, &community_dist, i_m.second,
                                                                 &time_us);
                    if (tmpset.size()<coreness)
                        continue;
                out_mutex.lock();
                    cnt++;
                    avr_Dist += community_dist;
                    avr_t += time_us;
                    out << std::to_string(i) << std::endl;
                    for (auto j: tmpset) {
                        out << std::to_string(cnns_2_ori[j]) << " ";
                    }
                    out << std::endl;
                    out << "Community Distance = " << community_dist << std::endl;
                    out << "Runtime : " << time_us << " mcrs" << std::endl;
                out_mutex.unlock();
                }
            avr_Dist /= cnt;
            avr_t /= cnt;

            std::cout << "Avr dist = " << avr_Dist << std::endl;
            std::cout << "Avr runtime = " << avr_t << std::endl;
            out << "Avr dist = " << avr_Dist << std::endl;
            out << "Avr runtime = " << avr_t << std::endl;
                out.close();
            } 
            
            if (run_wo > 0) {
                Query_More_Nodes(queryS, wo_path, coreness, d, gamma);
            }

            return 0;
        }

        template<typename _stl>
        CSTATUS Del_Nodes_Truss(const adj_t &ind_adj, truss_decomposition &cur_truss, std::vector<CCORE> &edge_support,
                                const std::unordered_map<CNODEID, CNODEID> &cnns_2_coreCur,
                                _stl &need_del, CCORE trussness) {
            std::vector<CNODEID> need_del_l;
            need_del_l.reserve(need_del.size());
            for (auto i = need_del.begin(); i != need_del.end(); ++i) {
                need_del_l.emplace_back(cnns_2_coreCur.find(*i)->second);
            }
            std::queue<CEDGEID> need_del_edge;

            // 0410 new
            for (auto cur_del: need_del_l) {
                auto &tmp_vec = ind_adj[cur_del];
                for (auto i = 0; i < tmp_vec.size(); ++i) {
                    for (auto j = i + 1; j < tmp_vec.size(); ++j) {
                        CNODEID v1 = tmp_vec[i];
                        CNODEID v2 = tmp_vec[j];
                        CEDGEID cur_edge_id = cur_truss.Find_Edge(v1, v2);
                        if (cur_edge_id + 1 != 0) {
                            need_del_edge.emplace(cur_edge_id);
                        }
                    }
                }
            }

            Del_Edge_Truss(ind_adj, cur_truss, edge_support, trussness, need_del_edge);
            return 0;
        }

        CSTATUS Del_Edge_Truss(const adj_t &ind_adj, truss_decomposition &cur_truss, std::vector<CCORE> &edge_support,
                               CCORE trussness, std::queue<CEDGEID> &need_del_edge) {
            auto Edge_deled = [&edge_support](CEDGEID cedgeid) { return edge_support[cedgeid] == -1; };
//            std::vector<bool > deleted(edge_support.size(), false);
            auto tmp_pair = std::make_pair(-1, -1);
            while (!need_del_edge.empty()) {
                auto i = need_del_edge.front();
                if (edge_support[i] == -1) {
                    need_del_edge.pop();
                    continue;
                }
                CNODEID v1 = cur_truss.edge_2_endpt[i].first;
                CNODEID v2 = cur_truss.edge_2_endpt[i].second;
//                cur_truss.edge_map.erase(std::make_pair(v1, v2));
//                cur_truss.edge_2_endpt[i] = tmp_pair;
                edge_support[i] = -1;

                std::vector<CNODEID> common_nei;
                std::set_intersection(ind_adj[v1].begin(), ind_adj[v1].end(), ind_adj[v2].begin(), ind_adj[v2].end(),
                                      std::inserter(common_nei, common_nei.begin()));
                for (auto v3: common_nei) {
                    CEDGEID e1 = cur_truss.Find_Edge(v1, v3);
                    CEDGEID e2 = cur_truss.Find_Edge(v2, v3);
                    if (Edge_deled(e1) || Edge_deled(e2))
                        continue;

                    if (edge_support[e1] > trussness - 2) {
                        edge_support[e1]--;
                    } else if (edge_support[e1] > 0) {
//                        edge_support[e1] = 0;
                        need_del_edge.emplace(e1);
                    }
                    if (edge_support[e2] > trussness - 2) {
                        edge_support[e2]--;
                    } else if (edge_support[e2] > 0) {
//                        edge_support[e2] = 0;
                        need_del_edge.emplace(e2);
                    }
                }
//                deleted[i] = true;
                need_del_edge.pop();
            }
            return 0;
        }

        std::set<CNODEID> ATC_Query_withIndex_Truss_Bulk(CNODEID queryid, CCORE req_trussness, int d, CDIST *com_dist,
                                                         std::vector<CNODEID> &CNNS_res, size_t *time_us) {
            std::set<CNODEID> ind_Vert;
            adj_t ind_adj;
            std::vector<CNODEID> coreCur_2_cnns;
            std::unordered_map<CNODEID, CNODEID> cnns_2_coreCur;
            //  CNNS id
            CNODEID queryID = queryid;
            std::set<CNODEID> CNNS_res_cnnsid;
            for (auto i: CNNS_res) {
                CNNS_res_cnnsid.emplace(ori_2_cnns_map[i]);
            }

            std::vector<CNODEID> filtered_ind_Vert;
            auto d_neighbors = Get_Loc_Temp_Graph(queryID, req_trussness, d);
            std::set_intersection(CNNS_res_cnnsid.begin(), CNNS_res_cnnsid.end(), d_neighbors.begin(),
                                  d_neighbors.end(),
                                  std::inserter(filtered_ind_Vert, filtered_ind_Vert.begin()));
            filtered_ind_Vert.emplace_back(queryID);
            std::vector<CNODEID > nodes_in_truss;
            nodes_in_truss = Truss_Test_Prune_Speedup( req_trussness, queryID,filtered_ind_Vert);
            std::set<CNODEID> nodes_set(nodes_in_truss.begin(), nodes_in_truss.end());
            if (nodes_in_truss.size() == 0){
                return {};
            }
            auto s = CLOCK;
            Init_Ind_Graph_CNNSId(nodes_in_truss, ind_adj, ind_Vert, coreCur_2_cnns, cnns_2_coreCur);
            auto e = CLOCK;
            *time_us += TIME_US(s,e);
            return ATC_Query_Truss_Bulk_General(queryID, req_trussness, com_dist, ind_adj, nodes_set,
                                                coreCur_2_cnns, cnns_2_coreCur, time_us);
        }

        template<typename stl>
        CSTATUS Init_Ind_Graph_CNNSId(const stl &input_nodes, adj_t &ind_adj, std::set<CNODEID> &ind_Vert,
                                      std::vector<CNODEID> &coreCur_2_cnns,
                                      std::unordered_map<CNODEID, CNODEID> &cnns_2_coreCur) {
            int cnt = 0;
            for (auto pi = input_nodes.begin(); pi != input_nodes.end(); ++pi) {
                auto i = *pi;
                CNODEID cur_cnns_id = i;
                ind_Vert.emplace(cur_cnns_id);
                coreCur_2_cnns.emplace_back(cur_cnns_id);
                cnns_2_coreCur.emplace(cur_cnns_id, cnt);
                cnt++;
            }

            for (auto v = 0; v < cnt; ++v) {
                CNODEID i = coreCur_2_cnns[v];
                std::set<CNODEID> tmp_set;
                std::vector<CNODEID> tmp_vec;
                for (auto j: cur_graph_ori_adj[i]) {
                    if (ind_Vert.count(j)) {
                        tmp_set.emplace(j);
                        tmp_vec.emplace_back(cnns_2_coreCur.find(j)->second);
                    }
                }
                ind_adj.emplace_back(tmp_vec);
            }
            return 0;
        }

        std::set<CNODEID>
        ATC_Query_Truss_Bulk_General(CNODEID queryid, CCORE req_trussness, CDIST *com_dist, adj_t &ind_adj,
                                     std::set<CNODEID> &ind_Vert, const std::vector<CNODEID> &coreCur_2_cnns,
                                     const std::unordered_map<CNODEID, CNODEID> &cnns_2_coreCur, size_t *time_us,
                                     float epsilon = 0.03) {
            int begin_sz = ind_adj.size();
            CNODEID queryID = queryid;
            truss_decomposition cur_truss(ind_adj, 1);
            std::vector<CCORE> support = cur_truss.sup;

            std::vector<CCORE> node_trussness = cur_truss.node_trussness;
            CNODEID query_truss_id = cnns_2_coreCur.find(queryID)->second;
            std::vector<CNODEID> query_related_edges;
            for (auto v: ind_adj[query_truss_id]) {
                query_related_edges.emplace_back(cur_truss.Find_Edge(query_truss_id, v));
            }

            auto s = CLOCK;
            bool flag = 1;
            std::vector<CNODEID> attr_cnt;
            attr_cnt.clear();
            attr_cnt.resize(numeric_num+text_num, 0);

            CDIST max_score = 0;
            std::bitset<numeric_num+text_num> Wq = Get_Acq_d(queryID);
//            std::set<CNODEID> deleted;
            CDIST cur_score = 0;
            cur_score = atcComDist_boost(ind_Vert, Wq, attr_cnt);
            max_score = cur_score;
            std::vector<CNODEID> need_del_l;
//            CNODEID need_del = -1;
            while (!ind_Vert.empty() && flag) {

                int need_del_num = std::max(1, int(epsilon * ind_Vert.size() / (1 + epsilon)));

                cur_score = atcComDist_boost1(ind_Vert, Wq, attr_cnt);
                if (cur_score >= max_score) {
                    max_score = cur_score;

                } else {
                    for (auto need_del: need_del_l) {
                        ind_Vert.emplace(need_del);
                    }
                    flag = false;
                    break;
                }

                need_del_l.clear();
                need_del_l.reserve(need_del_num);
                CDIST contri = 100000000;
                maxHeap<CDIST> contri_heap;
                for (auto i: ind_Vert) {
                    CDIST tmp_contri = atcNodeContri_boost(ind_Vert, i, Wq, attr_cnt);
                    contri_heap.emplace(-tmp_contri, i);
                }

                for (auto i = 0; i < need_del_num; ++i) {
                    need_del_l.emplace_back(contri_heap.top().second);
                    contri_heap.pop();
                }
                Del_Nodes_Truss(ind_adj, cur_truss, support, cnns_2_coreCur, need_del_l, req_trussness);
                CCORE query_trussness = 0;
                for (auto e: query_related_edges) {
                    query_trussness = std::max(support[e] + 2, query_trussness);
                }
                if (query_trussness < req_trussness) {
                    flag = false;
                    break;
                }

                for (auto need_del: need_del_l) {
                    for (auto i = Get_Acq_d(need_del)._Find_first(); i != numeric_num+text_num; i = Get_Acq_d(need_del)._Find_next(i)) {
                        attr_cnt[i]--;
                    }
                    ind_Vert.erase(need_del);
                }
            }
            auto e = CLOCK;
            *time_us += TIME_US(s, e);
            *com_dist = max_score;
            // *com_dist =  atcComDist_boost(ind_Vert, Wq, attr_cnt);
            assert(ind_Vert.count(queryID));
            int end_sz = ind_Vert.size();
            return ind_Vert; //  这里return的是cnns图ID
        }

        std::set<CNODEID>
        ATC_Query_Truss_Loc_Size_Bounded_kd(CNODEID queryid, CCORE req_trussness, int d, CDIST *com_dist, int max_size,
                                            size_t *time_us) {
            //  CNNS id
            CNODEID queryID = queryid;
            auto &cur_data = Get_Acq_d(queryID);
            std::vector<int> keywords;
            for (int i = cur_data._Find_first(); i < numeric_num+text_num; i = cur_data._Find_next(i)) {
                keywords.emplace_back(i);
            }
//            std::set<CNODEID> init_graph =  Get_Loc_Temp_Graph(queryid, keywords);
            auto s = CLOCK;
            std::set<CNODEID> init_graph = Get_Loc_Temp_Graph_Size_Bounded(queryid, keywords, max_size, req_trussness, d);

            auto e = CLOCK;
            *time_us += TIME_US(s,e);
            std::vector<CNODEID> init_vec(init_graph.begin(), init_graph.end());
            auto nodes_in_truss = Truss_Test_Prune_Speedup( req_trussness, queryID,init_vec);
//            std::set<CNODEID> init_graph =  Get_Loc_Temp_Graph(queryid, keywords);

            std::set<CNODEID> ind_Vert;
            adj_t ind_adj;
            std::vector<CNODEID> coreCur_2_cnns;
            std::unordered_map<CNODEID, CNODEID> cnns_2_coreCur;
                
            ind_Vert.emplace(queryID);
//            Init_Ind_Graph_CNNSId(init_graph, ind_adj, ind_Vert, coreCur_2_cnns, cnns_2_coreCur);
            s = CLOCK;
            Init_Ind_Graph_CNNSId(Nodes_Filter(nodes_in_truss, req_trussness), ind_adj, ind_Vert, coreCur_2_cnns,
                                  cnns_2_coreCur);
            e = CLOCK;
            *time_us += TIME_US(s,e);
        if (ind_Vert.size() < req_trussness - 1){
            return {};
        }
//            return ATC_Query_Truss_Basic_General(queryID, req_trussness, com_dist, ind_adj, ind_Vert, coreCur_2_cnns, cnns_2_coreCur, time_us);
            return ATC_Query_Truss_Bulk_General(queryID, req_trussness, com_dist, ind_adj, ind_Vert, coreCur_2_cnns,
                                                cnns_2_coreCur, time_us);
        }

        // BFS d-graph
        std::set<CNODEID> Get_Loc_Temp_Graph(const CNODEID queryid, int coreness, int d) {
            //  queryid : CNNS_id
            std::set<CNODEID> res_graph;
            std::queue<CNODEID> bfs_queue;
            bfs_queue.emplace(queryid);

            int hop = 0;
            int cur_expand_cnt = 0;
            int need_expand_cnt = 1;
            int need_next_expand_cnt = 0;
            while(!bfs_queue.empty() && hop <= d){
                if (cur_expand_cnt++ < need_expand_cnt){
                    auto cur_expand = bfs_queue.front();
                    for (auto i : cur_graph_ori_adj[cur_expand]){
                        if (core_list[i] >= coreness) {
                            auto nei_iter = res_graph.find(i);
                            if (nei_iter == res_graph.end()) {
                                need_next_expand_cnt++;
                                res_graph.emplace(i);
                                if (hop < d)
                                    bfs_queue.emplace(i);
                            }
                        }
                    }
                    bfs_queue.pop();
                } else {
                    hop++;
                    need_expand_cnt = need_next_expand_cnt;
                    cur_expand_cnt = 0;
                    need_next_expand_cnt = 0;
                }
            }
            return res_graph;
        }

        std::set<CNODEID>
        Get_Loc_Temp_Graph_Size_Bounded(const CNODEID queryid, const std::vector<int> &keywords, int max_size,
                                        CCORE coreness, int d) {
            // node_id, hop
            std::set<CNODEID> res_graph;
            std::vector<CNODEID> keywords_cnt(keywords.size(), 0);
            std::bitset<numeric_num+text_num> cur_data;
            for (auto i: keywords) {
                cur_data.set(i);
            }
            std::vector<std::vector<CNODEID> > candi_per_hop(d + 1, std::vector<CNODEID>());
            std::vector<bool> vis(ntotal_, false);
            vis[queryid] = true;
            candi_per_hop[0].emplace_back(queryid);

            std::bitset<numeric_num+text_num> Wq;
            for (auto kw = 0; kw < keywords.size(); ++kw) {
                Wq.set(keywords[kw]);
            }

            for (auto i = 0; i <= d; ++i) {
                std::vector<bool> cur_added(candi_per_hop[i].size(), false);
                bool continue_iter = false;
                do {
                    continue_iter = false;
                    float max_contri = 0;
                    int max_idx = -1;
                    for (auto idx = 0; idx < candi_per_hop[i].size(); ++idx) {
                        if (cur_added[idx])
                            continue;
                        CNODEID node = candi_per_hop[i][idx];
                        float cur_contri = Cal_Contri(node, res_graph, keywords, keywords_cnt);
                        if (cur_contri > max_contri) {
                            max_contri = cur_contri;
                            max_idx = idx;
                        }
                    }
                    float f_Gt_Wq = atcComDist_boost_loosecnt(res_graph, Wq, keywords_cnt);
                    if (max_contri >= f_Gt_Wq) {
                        continue_iter = true;
                    }
                    if (continue_iter) {
                        auto max_node = candi_per_hop[i][max_idx];
                        res_graph.emplace(max_node);
                        if (res_graph.size() >= max_size) {
                            break;
                        }
                        cur_added[max_idx] = true;
                        std::bitset<numeric_num+text_num> &tmp_bitset = Get_Acq_d(max_node);
                        for (auto j = 0; j < keywords.size(); ++j) {
                            if (tmp_bitset.test(keywords[j])) {
                                keywords_cnt[j]++;
                            }
                        }
                        if (i < d) {
                            for (auto next_expand: cur_graph_ori_adj[max_node]) {
                                if (!vis[next_expand] && core_list[next_expand] >= coreness) {
                                    candi_per_hop[i + 1].emplace_back(next_expand);
                                    vis[next_expand] = true;
                                }
                            }
                        }
                    }
                } while (continue_iter);
            }
            return res_graph;
        }


        float Cal_Contri(const CNODEID cur_id, const std::set<CNODEID> &tmp_graph, const std::vector<int> &keywords,
                         const std::vector<CNODEID> &keywords_cnt) {
            if (tmp_graph.size() == 0) {
                return true;
            }
            std::bitset<numeric_num+text_num> &cur_keyw = Get_Acq_d(cur_id);
            std::bitset<numeric_num+text_num> Wq;
            std::vector<CNODEID> keywords_cnt_addcur(keywords_cnt);

            float cnt = 0;
            for (auto kw = 0; kw < keywords.size(); ++kw) {
                Wq.set(keywords[kw]);
                if (!cur_keyw.test(keywords[kw]))
                    continue;
                keywords_cnt_addcur[kw]++;
                cnt += keywords_cnt[kw];
            }
            cnt *= 2;
            return cnt;
        }

        template<typename stl>
        std::set<CNODEID> Nodes_Filter(stl container, CCORE req_trussness) {
            std::set<CNODEID> res;
            for (auto i = container.begin(); i != container.end(); ++i) {
                if (core_list[*i] >= req_trussness) {
                    res.emplace(*i);
                }
            }
            return res;
        }
        inline std::bitset<numeric_num+text_num> &Get_Acq_d(CNODEID A) {
            return atc_data_[A];
        }

        CDIST
        atcNodeContri_boost(const std::set<CNODEID> &community, const CNODEID target_node, const std::bitset<numeric_num+text_num> &Wq,
                            std::vector<CNODEID> &attr_cnt) {
            CDIST res = 0;
            auto intersec = Get_Acq_d(target_node) & Wq;
            std::vector<CNODEID> track; //  记录哪些位需要记录
            for (auto i = 0; i < numeric_num + text_num; ++i) {
      if (intersec[i]) {
        track.emplace_back(i);
      }
    }
    for (auto i : track) {
      res += attr_cnt[i] * 2 - 1;
    }

    return res;
  }

  CDIST atcComDist_boost1(const std::set<CNODEID> &community,
                          const std::bitset<numeric_num + text_num> &Wq,
                          const std::vector<CNODEID> &attr_cnt) {
    if (community.size() == 0) {
      return 0;
    }
    CDIST res = 0;
    std::vector<int> keywords;
    for (auto i = Wq._Find_first(); i < numeric_num + text_num;
         i = Wq._Find_next(i)) {
      keywords.emplace_back(i);
    }

    for (auto i = 0; i < keywords.size(); ++i) {
      res += attr_cnt[keywords[i]] * attr_cnt[keywords[i]] * 1.0;
    }

    res /= community.size();
    return res;
  }

  CDIST atcComDist_boost_loosecnt(const std::set<CNODEID> &community,
                                  const std::bitset<numeric_num + text_num> &Wq,
                                  const std::vector<CNODEID> &attr_cnt) {
    if (community.size() == 0) {
      return 0;
    }
    CDIST res = 0;
    std::vector<int> keywords;
    for (auto i = Wq._Find_first(); i < numeric_num + text_num;
         i = Wq._Find_next(i)) {
      keywords.emplace_back(i);
    }

    for (auto i = 0; i < keywords.size(); ++i) {
      res += attr_cnt[i] * attr_cnt[i] * 1.0;
    }

    res /= community.size();
    return res;
  }

  CDIST atcComDist_boost(const std::set<CNODEID> &community,
                         const std::bitset<numeric_num + text_num> &Wq,
                         std::vector<CNODEID> &attr_cnt) {
    CDIST res = 0;
    std::vector<int> keywords;
    for (auto i = Wq._Find_first(); i != numeric_num + text_num;
         i = Wq._Find_next(i)) {
      keywords.emplace_back(i);
    }
    std::vector<int> keywords_cnt(keywords.size(), 0);
    for (auto j : community) {
      std::bitset<numeric_num + text_num> &inter = Get_Acq_d(j);
      int ll = inter._Find_first();
      for (auto i = 0; i < keywords.size(); ++i) {
        if (inter[keywords[i]])
          keywords_cnt[i]++;
      }
    }
    for (auto i = 0; i < keywords.size(); ++i) {
      attr_cnt[keywords[i]] = keywords_cnt[i];
      res += keywords_cnt[i] * keywords_cnt[i] * 1.0;
    }
    res /= community.size();
    return res;
  }
  inline bool In_Same_Compo(CNODEID queryID, CCORE coreness, CNODEID cnns_ID) {
    return ((core_list[cnns_ID] >= coreness) &&
            (Get_Current_Component_By_Cnns_Id(cnns_ID, coreness) ==
             Get_Current_Component_By_Cnns_Id(queryID, coreness)));
  }

  std::vector<CNODEID> Truss_Test_Prune_Speedup(chci::CCORE corenum,
                                                chci::CNODEID queryID,
                                                std::vector<chci::CNODEID> &a) {
    std::vector<CNODEID> ind_V;
    flat_hash_map<CNODEID, flat_hash_set<CNODEID>> ind_G;
    std::queue<CNODEID> delete_l;

    std::vector<CNODEID> &coreCur_2_Ori = a;
    std::sort(coreCur_2_Ori.begin(), coreCur_2_Ori.end());
    flat_hash_map<CNODEID, flat_hash_set<CNODEID>> coreOri_2_Cur;
    CNODEID a_total = a.size();

    std::vector<CNODEID> truss_2_cnns;
    flat_hash_map<CNODEID, CNODEID> cnns_2_truss;
    int truss_sz = 0;
    for (auto i = 0; i < a_total; ++i) {
      flat_hash_set<CNODEID> cur_neigh;
      CNODEID cur_cnns_id = coreCur_2_Ori[i];

      auto &cur_adj = cur_graph_ori_adj[cur_cnns_id];
      //            std::sort(cur_adj.begin(), cur_adj.end());
      std::set_intersection(cur_adj.begin(), cur_adj.end(),
                            coreCur_2_Ori.begin(), coreCur_2_Ori.end(),
                            std::insert_iterator<flat_hash_set<CNODEID>>(
                                cur_neigh, cur_neigh.begin()));
      if (cur_neigh.size() >= corenum - 1) {
        truss_2_cnns.emplace_back(cur_cnns_id);
        cnns_2_truss[cur_cnns_id] = truss_sz;
        ind_G[cur_cnns_id] = cur_neigh;
        truss_sz++;
      }
    }
    std::vector<std::vector<CNODEID>> start_G(truss_sz,
                                              std::vector<CNODEID>(0));
    for (auto i = 0; i < truss_sz; ++i) {
      start_G[i].reserve(ind_G[truss_2_cnns[i]].size());
      for (auto nei : ind_G[truss_2_cnns[i]]) {
        if (cnns_2_truss.contains(nei))
          start_G[i].emplace_back(cnns_2_truss[nei]);
      }
    }

    if (start_G.size() < corenum - 1 || !cnns_2_truss.contains(queryID)) {
      std::vector<CNODEID> null_vec;
      return null_vec;
    }

    truss_decomposition trussDecomp(start_G);
    //        trussDecomp.Parallel_K_Truss_Decomp(8);
    auto res_test_set =
        trussDecomp.Judge_K_Truss(cnns_2_truss[queryID], corenum);
    //        core_decomposition<CNODEID> coreDecomp(start_G,
    //        coreCur_2_Ori.size());
    auto corel = trussDecomp.node_trussness;
    if (res_test_set.empty()) {
    } else {
      ind_V.reserve(res_test_set.size());
      for (auto node : res_test_set) {
        ind_V.emplace_back(truss_2_cnns[node]);
      }
    }
    return ind_V;
  }
};

}

#endif
