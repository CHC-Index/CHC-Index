#ifndef READ_ADJ_H_VAC_H
#define READ_ADJ_H_VAC_H

#include <iostream>
#include "../include/chci.h"
#include "../include/basic_define.h"
#include "phmap.h"
#include <ctime>

using phmap::flat_hash_map;
using phmap::parallel_flat_hash_map;
using phmap::flat_hash_set;
using phmap::parallel_flat_hash_set;
namespace chci {

    template<int numeric_num, int text_num>
    class vac : public chci::chci_graph<numeric_num, text_num> {
    public:
        int search_threads = 1;
        std::mutex truss_mutex;
        using chci_graph<numeric_num, text_num>::core_list;
        using chci_graph<numeric_num, text_num>::cnns_2_ori;
        using chci_graph<numeric_num, text_num>::node_num_per_compo;
        using chci_graph<numeric_num, text_num>::ms_;
        using chci_graph<numeric_num, text_num>::Get_Current_Component_By_Cnns_Id;
        using chci_graph<numeric_num, text_num>::Load_Graph_bin;
        using chci_graph<numeric_num, text_num>::Load_Graph_bin_part;
        using chci_graph<numeric_num, text_num>::ori_2_cnns_map;
        using chci_graph<numeric_num, text_num>::cur_graph_ori_adj;
        using chci_graph<numeric_num, text_num>::ntotal_;
        using chci_graph<numeric_num, text_num>::data_;

        vac() = default;

        vac(const CCHAR *graph_path, const CCHAR *vertex_path, const CCHAR *data_path, const CCHAR *edge_path,
            Dataset_Type speci_dataset, StructureMethod sMethod, Cmodel com_base, float alpha, float B,
            CUINT buildQueueLength, CDEGREE maxDegree, CUINT threadNum)
                : chci_graph<numeric_num, text_num>(graph_path, vertex_path, data_path, edge_path, speci_dataset, TEXT, sMethod, sMethod, com_base,
                                                    alpha, buildQueueLength, maxDegree, threadNum) { }

        void Load_Index(Dataset_Type ds_t, const std::string& cnns_index_input, const std::string& data_path, const std::string &graph_path,
                        const std::string &vertex_path, const std::string &edge_path) {
            Load_Graph_bin(ds_t, HYB, truss, baseline, cnns_index_input, data_path, graph_path, vertex_path, edge_path);
        }

        inline bool In_Same_Compo(CNODEID queryID, CCORE coreness, CNODEID cnns_ID) {
            return ((core_list[cnns_ID] >= coreness) &&
                    (Get_Current_Component_By_Cnns_Id(cnns_ID, coreness) ==
                     Get_Current_Component_By_Cnns_Id(queryID, coreness)));
        }

        CSTATUS Query_More_Nodes(std::vector<CNODEID> querySet, std::string res_path, CCORE coreness) {

            omp_set_num_threads(search_threads);
            std::ofstream out(res_path);
            std::mutex out_mutex;
            CDIST avr_Dist = 0;
            double avr_t = 0;
            int cnt = 0;
#pragma omp parallel for schedule(dynamic, 1)
            for (auto ii = 0; ii < querySet.size(); ++ii) {

                auto &i = querySet[ii];
                CDIST community_dist = 0;
                CNODEID query_CNNSID = i;
if (core_list[query_CNNSID] < coreness){
                    continue;
                }
#pragma omp critical
                {
                    cnt++;
                }
                size_t time_us = 0;
                auto tmpset = VAC_Query(query_CNNSID, coreness, &community_dist, &time_us);
                CDIST cur_com_dist = Community_Dist(tmpset);
                out_mutex.lock();
                avr_t += time_us;
                avr_Dist += cur_com_dist;
                out << "Node " << std::to_string(cnns_2_ori[query_CNNSID]) << std::endl;
                for (auto j: tmpset) {
                    out << std::to_string(cnns_2_ori[j]) << " ";
                }
                out << std::endl;
                out << "Dist = " << cur_com_dist << std::endl;
                out << "Runtime : " << time_us << " mcrs";
                out << std::endl;
                out_mutex.unlock();
            }

            avr_Dist /= cnt;
            avr_t /= cnt;
            out << "Avr dist = " << avr_Dist << std::endl;
            out << "Avr runtime = " << avr_t << " us." << std::endl;
            return 0;
        }

        CSTATUS Query_More_Nodes_withIndex_Formal(const std::string &CNNS_Res, std::string &res_path, std::string &res_path_wo, CCORE coreness,
                                                  int threads, bool run_with = false, bool run_wo = false) {
            search_threads = threads;
            omp_set_num_threads(search_threads);
            std::mutex out_mutex;

            std::ifstream in(CNNS_Res);
            if (!in) {
                std::cerr << "Search Result not valid!!!" << std::endl;
                return -9999;
            }
            std::istringstream in_s;

            // The ID of the vertex here has been transformed.
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
                //  chci_id -> vector<ori_id>
                query_with_cnnsRes.emplace_back(ori_2_cnns_map.find(tmp_id)->second, tmp_res);
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

            std::ofstream out;
            if (run_with) {
                out.open(res_path);
#pragma omp parallel for schedule(dynamic)
                for (auto ii = 0; ii < query_with_cnnsRes.size(); ++ii) {
                    auto &i_m = query_with_cnnsRes[ii];
                    CNODEID i = i_m.first;
                    CDIST community_dist = 0;
                    CNODEID query_CNNSID = i_m.first;
if (core_list[query_CNNSID] < coreness){
                    continue;
                }
                    size_t cur_time_us = 0;
                    auto tmpset = VAC_Query_withIndex(query_CNNSID, coreness, &community_dist, i_m.second,
                                                      &cur_time_us);
                    if (tmpset.size() < coreness){
                        continue;
                    }
#pragma omp critical
                    {
                        cnt++;
                    }
                    CDIST cur_com_dist = Community_Dist(tmpset);
                    std::unique_lock<std::mutex> out_lock(out_mutex);

                    avr_t += cur_time_us;

                    avr_Dist += cur_com_dist;
                    out << "Node " << std::to_string(cnns_2_ori[query_CNNSID]) << std::endl;
                    for (auto j: tmpset) {
                        out << std::to_string(cnns_2_ori[j]) << " ";
                    }
                    out << std::endl;
                    out << "Dist = " << cur_com_dist << std::endl;
                    out << "Runtime : " << cur_time_us << " us";
                    out << std::endl;
                }
                avr_Dist /= cnt;
                avr_t /= cnt;
                std::cout << "Avr dist = " << avr_Dist << std::endl;
                std::cout << "Avr runtime = " << avr_t << std::endl;
                out << "Avr dist = " << avr_Dist << std::endl;
                out << "Avr runtime = " << avr_t << " us." << std::endl;
                out.close();
            }

            if (run_wo) {
                Query_More_Nodes(queryS, res_path_wo, coreness);
            }

            return 0;
        }

        flat_hash_set<CNODEID> VAC_Query(CNODEID queryID, CCORE coreness, CDIST *com_dist, size_t *time_us) {
                    return VAC_Query_Truss(queryID, coreness, com_dist, time_us);
        }

        //  ind_Vert 存cnns id
        template<typename stl>
        CSTATUS Init_Ind_Graph_CNNSId(const stl &input_nodes, adj_t &ind_adj, flat_hash_set<CNODEID> &ind_Vert,
                                      std::vector<CNODEID> &coreCur_2_cnns,
                                      flat_hash_map<CNODEID, CNODEID> &cnns_2_coreCur) {
            int cnt = 0;
            for (auto pi = input_nodes.begin(); pi != input_nodes.end(); ++pi) {
                CNODEID cur_cnns_id = *pi;
                ind_Vert.emplace(cur_cnns_id);
                coreCur_2_cnns.emplace_back(cur_cnns_id);
                cnns_2_coreCur.emplace(cur_cnns_id, cnt);
                cnt++;
            }

            for (auto v = 0; v < cnt; ++v) {
                CNODEID i = coreCur_2_cnns[v];
                flat_hash_set<CNODEID> tmp_set;
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

        flat_hash_set<CNODEID>
        VAC_Query_Truss(CNODEID queryid, CCORE req_trussness, CDIST *com_dist, size_t *time_us) {
            //  CNNS id
            CNODEID queryID = queryid;
            std::set<CNODEID> init_graph;
            for (auto i = 0; i < ntotal_; ++i) {
                if (In_Same_Compo(queryID,req_trussness, i))
                    init_graph.emplace(i);
            }

            flat_hash_set<CNODEID> ind_Vert;
            adj_t ind_adj;
            std::vector<CNODEID> coreCur_2_cnns;
            flat_hash_map<CNODEID, CNODEID> cnns_2_coreCur;

            ind_Vert.emplace(queryID);
            Init_Ind_Graph_CNNSId(init_graph, ind_adj, ind_Vert, coreCur_2_cnns, cnns_2_coreCur);
            return VAC_Query_Truss_General(queryID, req_trussness, com_dist, ind_adj, ind_Vert, coreCur_2_cnns,
                                           cnns_2_coreCur, time_us);

        }

        //  need_del是cnns id
        CSTATUS Del_Node_Truss(const adj_t &ind_adj, truss_decomposition &cur_truss, std::vector<CCORE> &edge_support,
                               const flat_hash_map<CNODEID, CNODEID> &cnns_2_coreCur,
                               CNODEID need_del, CCORE trussness) {
            //   edge_support == 0 : deleted
            auto cur_del = cnns_2_coreCur.find(need_del)->second;
            auto &tmp_vec = ind_adj[cur_del];
            std::queue<CEDGEID> need_del_edge;

            for (auto j: tmp_vec) {
                need_del_edge.emplace(cur_truss.Find_Edge(j, cur_del));
            }
            Del_Edge_Truss(ind_adj, cur_truss, edge_support, trussness, need_del_edge);

            return 0;
        }

        CSTATUS Del_Edge_Truss(const adj_t &ind_adj, truss_decomposition &cur_truss, std::vector<CCORE> &edge_support,
                               CCORE trussness, std::queue<CEDGEID> &need_del_edge) {
            auto Edge_deled = [&edge_support](CEDGEID cedgeid) { return edge_support[cedgeid] == -1; };
            while (!need_del_edge.empty()) {
                auto i = need_del_edge.front();
                if (edge_support[i] == -1) {
                    need_del_edge.pop();
                    continue;
                }
                CNODEID v1 = cur_truss.edge_2_endpt[i].first;
                CNODEID v2 = cur_truss.edge_2_endpt[i].second;
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
                        need_del_edge.emplace(e1);
                    }
                    if (edge_support[e2] > trussness - 2) {
                        edge_support[e2]--;
                    } else if (edge_support[e2] > 0) {
                        need_del_edge.emplace(e2);
                    }
                }
                need_del_edge.pop();
            }
            return 0;
        }


        flat_hash_set<CNODEID>
        VAC_Query_Truss_General(CNODEID queryid, CCORE req_trussness, CDIST *com_dist, adj_t &ind_adj,
                                flat_hash_set<CNODEID> &ind_Vert, const std::vector<CNODEID> &coreCur_2_cnns,
                                const flat_hash_map<CNODEID, CNODEID> &cnns_2_coreCur, size_t *time_us) {

            CNODEID queryID = queryid;
            std::unique_lock<std::mutex> lock(truss_mutex);
            truss_decomposition cur_truss(ind_adj, 1);
            lock.unlock();
            std::vector<CCORE> support = cur_truss.sup;
            CNODEID query_truss_id = cnns_2_coreCur.find(queryID)->second;
            std::vector<CEDGEID> query_related_edges;
            for (auto v: ind_adj[query_truss_id]) {
                query_related_edges.emplace_back(cur_truss.Find_Edge(query_truss_id, v));
            }

            auto s = CLOCK;
            maxHeap<CDIST> dist_heap;
            for (auto cur_cnns_id: coreCur_2_cnns) {
                auto dis = ms_.full_dist_noNeigh(data_[queryID], data_[cur_cnns_id]);
                dist_heap.emplace(dis, cur_cnns_id);
            }

            bool flag = 1;

            CDIST max_score = 0;
            CDIST cur_score = 0;
            CNODEID need_del = -1;
            while (!ind_Vert.empty() && flag) {

                while (!ind_Vert.contains(dist_heap.top().second)) {
                    dist_heap.pop();
                }
                need_del = dist_heap.top().second;
                dist_heap.pop();

                CDIST contri = 100000000;

                Del_Node_Truss(ind_adj, cur_truss, support, cnns_2_coreCur, need_del, req_trussness);
                CCORE query_trussness = 0;
                for (auto e: query_related_edges) {
                    query_trussness = std::max(support[e] + 2, query_trussness);
                }
                if (query_trussness < req_trussness) {
                    flag = false;
                    break;
                }

                ind_Vert.erase(need_del);
            }
            auto e = CLOCK;
            *time_us = TIME_US(s, e);
            *com_dist = max_score;
            assert(ind_Vert.count(queryID));
            return ind_Vert; //  这里return的是cnns图ID
        }

  flat_hash_set<CNODEID> VAC_Query_withIndex(CNODEID queryID, CCORE coreness,
                                             CDIST *com_dist,
                                             std::vector<CNODEID> &CNNS_res,
                                             size_t *time_us) {
    return VAC_Query_withIndex_Truss_Speedup(queryID, coreness, com_dist,
                                             CNNS_res, time_us);
  }

  flat_hash_set<CNODEID> VAC_Query_withIndex_Truss_Speedup(
      CNODEID queryID, CCORE req_trussness, CDIST *com_dist,
      std::vector<CNODEID> &CNNS_res, size_t *time_us) {

    //  CNNS id
    flat_hash_set<CNODEID> init_graph;
    for (auto i : CNNS_res) {
      init_graph.emplace(ori_2_cnns_map.find(i)->second);
    }
    init_graph.emplace(queryID);

    flat_hash_set<CNODEID> ind_Vert;
    adj_t ind_adj;
    std::vector<CNODEID> coreCur_2_cnns;
    flat_hash_map<CNODEID, CNODEID> cnns_2_coreCur;

    ind_Vert.emplace(queryID);
    std::vector<CNODEID> tmp_Vert_vec(init_graph.begin(), init_graph.end());

    std::vector<CNODEID> nodes_in_truss;
    nodes_in_truss =
        Truss_Test_Prune_Speedup(req_trussness, queryID, tmp_Vert_vec);
    if (nodes_in_truss.size() == 0) {
      return {};
    }
    flat_hash_set<CNODEID> nodes_set(nodes_in_truss.begin(),
                                     nodes_in_truss.end());

    Init_Ind_Graph_CNNSId(nodes_set, ind_adj, ind_Vert, coreCur_2_cnns,
                          cnns_2_coreCur);
    return VAC_Query_Truss_General(queryID, req_trussness, com_dist, ind_adj,
                                   nodes_set, coreCur_2_cnns, cnns_2_coreCur,
                                   time_us);
  }

  template <typename _stl> CDIST Community_Dist(_stl community) {
    std::vector<CNODEID> cur_com;
    for (auto i : community) {
      cur_com.emplace_back(i);
    }
    CDIST max_dist = 0;
    for (auto i = 0; i < cur_com.size() - 1; ++i) {
      for (auto j = i + 1; j < cur_com.size(); ++j) {
        CDIST cur = ms_.full_dist_noNeigh(data_[cur_com[i]], data_[cur_com[j]]);
        if (max_dist < cur) {
          max_dist = cur;
        }
      }
    }
    return max_dist;
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
    auto res_test_set =
        trussDecomp.Judge_K_Truss(cnns_2_truss[queryID], corenum);
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

} // namespace cnns
#endif // READ_ADJ_H_VAC_H
