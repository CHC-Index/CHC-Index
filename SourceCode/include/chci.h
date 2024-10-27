//  This document contains the main code of CHC-Index, including index building and recall vertices.
#ifndef CS_ON_ANNS_CNNS_GRAPH_H
#define CS_ON_ANNS_CNNS_GRAPH_H

#include <vector>
#include <cstdint>
#include <cassert>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <random>
#include <memory>
#include <phmap.h>
#include "omp.h"

#include "basic_define.h"
#include "utils.h"
#include "metric_space.h"
#include "decomposition.h"
#include "unordered_map"
#include "basic_data.h"
#include "load_dataset_topology.h"

using phmap::flat_hash_map;
using phmap::parallel_flat_hash_map;
using phmap::flat_hash_set;
using phmap::parallel_flat_hash_set;


namespace chci {
    class CHCIndex {
    public:
        std::vector<std::vector<std::pair<CLAYER, CNODEID> > > hier_adj;
        std::vector<CCORE> core_list;
        parallel_flat_hash_map<CCORE, CLAYER> real_core_2_layer_map;
        std::vector<CNODEID> sp_list;
        adj_t sp_l_list;
        adj_t ori_graph_adj;
        adj_t cur_graph_ori_adj;
        std::vector<CNODEID> cnns_2_ori;
        parallel_flat_hash_map<CNODEID, CNODEID> ori_2_cnns_map;
        std::vector<CNODEID> cnns_2_reordered;
        std::vector<CNODEID> reordered_2_cnns;
        std::vector<std::vector<CNODEID>> node_num_per_compo;
        std::vector<CDEGREE> deg_list;
        std::vector<std::vector<int >> compo_l;
        float alpha_ = 1.2;
        float beta_ = -1;
        float Paper_A = -1;
        float rp_lb_ = -1;
        float rp_ub_ = -1;
        CLAYER layer_num = 0;
        CUINT build_queue_length = 64;
        CUINT search_queue_length = 64;
        CDEGREE max_degree = 24;
        CUINT numThreads = 1;
        bool enable_rppp = true;
        CDIST gamma = 2;
        float L_scale = 1.2;
        
 // 为了增删改实验引入的数据结构
        adj_t cur_graph_ori_adj_complete;
        std::set<CNODEID> valid_nodes;

        virtual void Metric_Interface(__Metric) = 0;

        virtual std::vector<CNODEID> Retrieve(CNODEID p, float r, CCORE cCORE, size_t *time) = 0;

        virtual void Build_Init(const std::string &graph_path,
                                const std::string &vertex_path,
                                const std::string &data_path,
                                const std::string &edge_path,
                                Dataset_Type speci_dataset,
                                __Metric _metric_,
                                StructureMethod sMethod,
                                Cmodel comBase,
                                float alpha,
                                CCONTINUOUS B,
                                CUINT buildQueueLength,
                                CDEGREE maxDegree,
                                CUINT threadNum,
                                bool enable_rppp) = 0;

        virtual void build_all_layer() = 0;

        virtual void Build_Init_part(const std::string &graph_path,
                                     const std::string &vertex_path,
                                     const std::string &data_path,
                                     const std::string &edge_path,
                                     const std::string &valid_node_path,
                                     Dataset_Type speci_dataset,
                                     __Metric _metric_,
                                     StructureMethod sMethod,
                                     Cmodel comBase,
                                     float alpha,
                                     CCONTINUOUS B,
                                     CUINT buildQueueLength,
                                     CDEGREE maxDegree,
                                     CUINT threadNum,
                                     bool rppp_enable) = 0;

        virtual CSTATUS build_all_layer_part() = 0;

        virtual void Save_Graph_bin(const std::string &cnns_index_output) = 0;

        virtual void Load_Graph_bin(Dataset_Type ds_t,
                                    __Metric mtc,
                                    Cmodel cmodel,
                                    StructureMethod sMeth,
                                    const std::string &cnns_index_input,
                                    const std::string &data_path,
                                    const std::string &graph_path,
                                    const std::string &vertex_path,
                                    const std::string &edge_path) = 0;

        virtual CSTATUS Batch_Add(const std::string &need_add_path) = 0;
        virtual CSTATUS Batch_Del(const std::string &need_del_path) = 0;
        virtual void Load_Graph_bin_part(Dataset_Type ds_t, __Metric mtc, Cmodel cmodel, StructureMethod sMeth,
                                         const std::string &cnns_index_input, const std::string &data_path,
                                         const std::string &graph_path,
                                         const std::string &vertex_path, const std::string &edge_path
                , const std::string &valid_node_path) = 0;

        virtual void Load_Graph_Topology(const std::string &graph_path,
                                         const std::string &vertex_path,
                                         const std::string &edge_path,
                                         const std::string &valid_node_path,
                                         Dataset_Type speci_dataset) = 0;
    };


    template<int numeric_num, int text_num>
    class chci_graph : public CHCIndex {
    public:
        chci_graph() = default;
        CNODEID ntotal_;
        CCORE max_coreness;
        std::vector<General_data<numeric_num, text_num>> data_;
        std::vector<Structure_data> struct_data_;
        __Metric metric_;
        HYBSpace<numeric_num, text_num> ms_;
        Dataset_Type specific_dataset;
        StructureMethod sMethod_;
        Cmodel communityBase;

        std::unordered_map<CNODEID, std::set<CNODEID>> GT_data;

        std::vector<std::mutex> link_list_locks;
        std::mutex cout_lock;
        std::mutex hier_adj_mutex;
        std::mutex adj_mutex;
        std::mutex struct_data_mutex;
        std::vector<std::mutex> compo_mutex;
        std::mutex core_list_mutex;
        std::mutex valid_nodes_mutex;
        std::mutex node_num_mutex;


        void Metric_Interface(__Metric in_metric) {
            ms_ = HYBSpace<numeric_num, text_num>(specific_dataset, in_metric);

        }

        CNODEID Find_Enter_Pt(CCORE cur_core, size_t cur_compo) {
            std::vector<CCONTINUOUS> centre(numeric_num + text_num, 0);
            int cnt = 0;
            for (auto i = 0; i < ntotal_; ++i) {
                if (core_list[i] >= cur_core && compo_l[i][cur_core] == cur_compo) {
                    for (auto j = 0; j < numeric_num; ++j) {
                        centre[j] += data_[i].numeric_data[j];
                    }
                    for (auto j = 0; j < text_num; ++j) {
                        centre[j] += data_[i].text_bin[j];
                    }
                }
                cnt++;
            }
            for (auto &i: centre) {
                i /= cnt;
            }
            CCONTINUOUS min_dist = 999;
            CNODEID enter_pt = -1;
            for (auto i = 0; i < ntotal_; ++i) {
                if (core_list[i] >= cur_core && compo_l[i][cur_core] == cur_compo) {
                    if (ms_.dist_Find_Enter_pt(centre, data_[i]) < min_dist) {
                        min_dist = ms_.dist_Find_Enter_pt(centre, data_[i]);
                        enter_pt = i;
                    }
                }
            }
            return enter_pt;
        }

        void Init_Nodes(const std::string &data_path) {
            omp_set_num_threads(numThreads);
            Init_data_(data_path); // initialize features
            
            // initialize structure features for opt2
            struct_data_.resize(ntotal_);

            if (sMethod_ != baseline) {
#pragma omp parallel for
                for (auto i = 0; i < ntotal_; ++i) {
                    CNODEID reordered_id = cnns_2_reordered[i];
                    CNODEID cnns_id = i;
                    struct_data_[i] = Structure_data{cnns_id, cur_graph_ori_adj[i], cnns_2_reordered};
                }
            }
        }

        void Init_data_(const std::string &data_path) {
            data_.resize(ntotal_);

            std::ifstream in(data_path);
            if (!in.is_open()) {
                throw std::runtime_error("Data path not valid !!!!!!!!");
            }
            std::stringstream buffer;
            buffer << in.rdbuf();
            in.close();
            std::string line;
            int count = 0;
            std::getline(buffer, line);
            while (std::getline(buffer, line)) {
                CNODEID cnns_id = -1;
                read_data_line(line, &cnns_id);
            }
        }

        int read_data_line(std::string &line, CNODEID *cnns_id_bak) {
            for (int i = 0; i < line.length(); i++) {
                if (line[i] == '[' || line[i] == ']')
                    line.erase(i, 1);
            }
            std::istringstream sin(line);
            CNODEID ori_id;
            sin >> ori_id;

            if (!ori_2_cnns_map.count(ori_id)) {
                sin.str("");
                sin.clear();
                return -1;
            }
            CNODEID cur_cnns_id = ori_2_cnns_map.find(ori_id)->second;
            *cnns_id_bak = cur_cnns_id;


            CCONTINUOUS cur_int = 0;
            std::bitset<text_num> _text_data(0);
            for (int i = 0; i < text_num; i++) {
                sin >> cur_int;
                if (cur_int > 0) {
                    _text_data.set(i);
                }
            }
            std::vector<CCONTINUOUS> _num_data(numeric_num, 0);
            for (auto i = 0; i < numeric_num; ++i) {
                sin >> _num_data[i];
            }
            data_[cur_cnns_id] = General_data<numeric_num, text_num>{_num_data, _text_data};
            return 0;
        }

        chci_graph(const std::string &graph_path,
                   const std::string &vertex_path,
                   const std::string &data_path,
                   const std::string &edge_path,
                   Dataset_Type speci_dataset,
                   __Metric _metric_,
                   StructureMethod sMethod,
                   Cmodel comBase,
                   float alpha,
                   CCONTINUOUS B,
                   CUINT buildQueueLength,
                   CDEGREE maxDegree,
                   CUINT threadNum) {
            specific_dataset = speci_dataset;
            ms_ = speci_dataset;
            sMethod_ = sMethod;
            communityBase = comBase;
            ntotal_ = 0;
            alpha_ = alpha;
            build_queue_length = buildQueueLength;
            max_degree = maxDegree;
            numThreads = threadNum;
            metric_ = _metric_;
            beta_ = B;
            Begin_Build_Graph(graph_path, vertex_path, data_path, edge_path, speci_dataset, sMethod);
        }

        void Build_Init(const std::string &graph_path,
                        const std::string &vertex_path,
                        const std::string &data_path,
                        const std::string &edge_path,
                        Dataset_Type speci_dataset,
                        __Metric _metric_,
                        StructureMethod sMethod,
                        Cmodel comBase,
                        float alpha,
                        CCONTINUOUS B,
                        CUINT buildQueueLength,
                        CDEGREE maxDegree,
                        CUINT threadNum,
                        bool rppp_enable) {
            specific_dataset = speci_dataset;
            ms_ = HYBSpace<numeric_num, text_num>(speci_dataset, _metric_);
            sMethod_ = sMethod;
            communityBase = comBase;
            ntotal_ = 0;
            alpha_ = alpha;
            build_queue_length = buildQueueLength;
            max_degree = maxDegree;
            numThreads = threadNum;
            metric_ = _metric_;
            beta_ = B;
            enable_rppp = rppp_enable;
            Begin_Build_Graph(graph_path, vertex_path, data_path, edge_path, speci_dataset, sMethod);
        }

        void Begin_Build_Graph(const std::string &graph_path,
                               const std::string &vertex_path,
                               const std::string &data_path,
                               const std::string &edge_path,
                               Dataset_Type speci_dataset,
                               StructureMethod structureMethod) {
            specific_dataset = speci_dataset;
            load_dataset_topology *ptmp_graph = new load_dataset_topology(graph_path, vertex_path, specific_dataset, edge_path);
            ntotal_ = ptmp_graph->total_node_num;
            cnns_2_ori = std::move(ptmp_graph->cnns_2_ori);
            ori_2_cnns_map = std::move(ptmp_graph->ori_2_cnns);
            ori_graph_adj = std::move(ptmp_graph->res_graph);
            cur_graph_ori_adj = std::move(ptmp_graph->cur_2_cur_adj);
            delete ptmp_graph;
            ntotal_ = cur_graph_ori_adj.size();
            hier_adj.resize(ntotal_);
            std::cout << "cur_graph_ori_adj size = " << cur_graph_ori_adj.size() << std::endl
                      << "ori_graph_adj size = " << ori_graph_adj.size()
                      << std::endl
                      << "ntotal_ = " << ntotal_ << std::endl;

            std::cout << "decomp begin." << std::endl;

            switch (communityBase) {
                case core: {
                    decomposition<CNODEID> *pcur_decomp = new decomposition<CNODEID>(cur_graph_ori_adj,
                                                                                     ntotal_);
                    core_list = std::move(pcur_decomp->BZ_Decomp());
                    delete pcur_decomp;
                    break;
                }
                case truss: {
                    truss_decomposition *pcur_decomp = new truss_decomposition(cur_graph_ori_adj);
                    pcur_decomp->Parallel_K_Truss_Decomp(numThreads);
                    core_list = std::move(pcur_decomp->node_trussness);
                    delete pcur_decomp;
                    break;
                }
            }

            for (auto i: core_list) {
                if (i > layer_num)
                    layer_num = i;
            }

            max_coreness = layer_num;
            layer_num++;
            deg_list = std::vector<CDEGREE>(layer_num, max_degree);
            std::vector<CNODEID> core_l_num(layer_num);
            for (auto i: core_list) {
                core_l_num[i] += 1;
            }
            for (auto i = core_l_num.size() - 1; i > 0; --i) {
                core_l_num[i - 1] += core_l_num[i];
            }

            for (auto i = 0; i < 10; ++i) {
                deg_list[i] = max_degree;
            }

            sp_list.resize(layer_num, -1);

            Core_Connective_4_Reorder();
            Core_Connective();
            Init_Nodes(data_path);

            link_list_locks = std::vector<std::mutex>(ntotal_);

            node_num_per_compo.resize(layer_num);
            for (auto i = 0; i < layer_num; ++i) {
                std::vector<CNODEID> empty(sp_l_list[i].size(), 0);
                node_num_per_compo[i].resize(sp_l_list[i].size());
            }
            for (auto &i: compo_l) {
                for (auto j = 0; j < i.size(); ++j) {
                    node_num_per_compo[j][i[j]]++;
                }
            }


            Paper_A = 1.0 - 1.0 / (alpha_ * alpha_);
            if (beta_ == 0) {
                rp_lb_ = 0;
                rp_ub_ = 9999999;
            } else {
                if (beta_ * beta_ - Paper_A < 0) {
                    throw std::runtime_error("Beta is not valid!!!");
                }
                rp_lb_ = beta_ - std::sqrt(beta_ * beta_ - Paper_A);
                rp_ub_ = beta_ + std::sqrt(beta_ * beta_ - Paper_A);
            }
            std::cout << "All information has been loaded successfully.\n";
        }

void Load_Graph_Topology(const std::string &graph_path,
                                 const std::string &vertex_path,
                                 const std::string &edge_path,
                                 const std::string &valid_node_path,
                                 Dataset_Type speci_dataset) {
    omp_set_num_threads(numThreads);
            specific_dataset = speci_dataset;
            load_dataset_topology *ptmp_graph = new load_dataset_topology(graph_path, vertex_path, specific_dataset, edge_path);
            ntotal_ = ptmp_graph->total_node_num;
            cnns_2_ori = std::move(ptmp_graph->cnns_2_ori);
            ori_2_cnns_map = std::move(ptmp_graph->ori_2_cnns);
            ori_graph_adj = std::move(ptmp_graph->res_graph);
            cur_graph_ori_adj = std::move(ptmp_graph->cur_2_cur_adj);
            cur_graph_ori_adj_complete = cur_graph_ori_adj;
            delete ptmp_graph;
            
            //  Read Valid Node
            std::ifstream valid_in(valid_node_path);
        if (valid_node_path == "null"){
            return;
        } else if (!valid_in){
                std::cerr << "NO VALID NDOES!!!!!!!!!!!!\n";
                return;
            }
            valid_nodes.clear();
//            std::set<CNODEID > valid_node;
            CNODEID tmp_ori_id;
            while (valid_in >> tmp_ori_id) {
                valid_nodes.emplace(ori_2_cnns_map[tmp_ori_id]);
            }
            // Induced Graph
            // Deal adjacency cur_graph_ori_adj
            std::vector<std::mutex> write_adj_mutex_vec(cur_graph_ori_adj.size());
#pragma omp parallel for num_threads(numThreads) schedule(dynamic)
            for (auto i = 0; i < ntotal_; ++i) {
//                CNODEID cur_ori_id = cnns_2_ori[i];
                if (valid_nodes.count(i)) {
                    std::vector<CNODEID> filtered_nei;
                    std::set_intersection(valid_nodes.begin(), valid_nodes.end(), cur_graph_ori_adj[i].begin(),
                                          cur_graph_ori_adj[i].end(), std::back_inserter(filtered_nei));
                    
                    std::unique_lock<std::mutex> lk1(write_adj_mutex_vec[i]);
                    cur_graph_ori_adj[i] = std::move(filtered_nei);
                } else {
                    std::unique_lock<std::mutex> lk1(write_adj_mutex_vec[i]);
                    cur_graph_ori_adj[i].clear();
                }
            }
            ntotal_ = cur_graph_ori_adj.size();
        }
        void Core_Connective_4_Reorder() {

            cnns_2_reordered.resize(ntotal_, 66666666);
            reordered_2_cnns.resize(ntotal_, 66666666);
            std::vector<bool> vis_reorder(ntotal_, false);
            std::vector<bool> vis(ntotal_, false);

            std::vector<std::vector<CNODEID>> reorder_layer;
            reorder_layer.resize(layer_num);
            int cnt_r = 0;
            for (int i = max_coreness; i >= 0; --i) {
                for (auto j = 0; j < ntotal_; ++j) {
                    if (core_list[j] >= i && !vis[j]) {
                        std::queue<CNODEID> tmp_Heap;
                        tmp_Heap.emplace(j);
                        vis[j] = true;
                        reordered_2_cnns[cnt_r] = j;
                        cnns_2_reordered[j] = cnt_r;
                        cnt_r++;

                        while (!tmp_Heap.empty()) {
                            auto cur_node = tmp_Heap.front();
                            tmp_Heap.pop();

                            for (auto k: cur_graph_ori_adj[cur_node]) {
                                if (vis[k]) {
                                    continue;
                                }
                                tmp_Heap.emplace(k);
                                reordered_2_cnns[cnt_r] = k;
                                cnns_2_reordered[k] = cnt_r;
                                cnt_r++;
                                vis[k] = true;
                            }
                        }
                    }
                }
            }
        }

        void build_all_layer() {
            build_1st_layer_comp();
            CLAYER cnt = 0;
            for (CLAYER i = 1; i < layer_num; i++) {
                build_one_layer_comp(i, ++cnt);
                real_core_2_layer_map.emplace(i, cnt);
            }
        }

        void build_1st_layer_comp() {

            omp_set_num_threads(numThreads);
            real_core_2_layer_map.emplace(0, 0);
            assert(ntotal_ > 0);
            std::vector<std::vector<CNODEID>> cur_adj;

#pragma omp parallel for schedule(dynamic)
            for (auto i = 0; i < sp_l_list[0].size(); ++i) {
                auto enter_pt = Find_Enter_Pt(0, i);
                sp_l_list[0][i] = enter_pt;
            }

            auto enter_pt = Find_Enter_Pt(0, 0);

            std::vector<std::vector<CNODEID>> zero_layer_adj(ntotal_);
            std::vector<CDEGREE> zero_layer_locked_num_l(ntotal_, 0);
            std::vector<CNODEID> zero_2_cnns;
            for (auto i = 0; i < ntotal_; i++) {
                zero_2_cnns.emplace_back(i);
            }
            NSW_Init_Roreder_Ver(0, 0, zero_layer_adj, zero_2_cnns, zero_2_cnns, enter_pt);
            CCONTINUOUS cnt = 0;
#pragma omp parallel for schedule(dynamic)
            for (CNODEID i = 0; i < ntotal_; i++) {

                CNODEID cnns_id = Get_Cnns_Id_By_Cur_Layer_Id(i, zero_2_cnns);
                size_t cur_compo = Get_Current_Component_By_Cnns_Id(cnns_id, 0);
                enter_pt = sp_l_list[0][cur_compo];
                CDEGREE cur_lock_num = 0;
                maxHeap<CDIST> neighbor_candi;
                Search_Neighbors_Build(cnns_id, enter_pt, 0, zero_layer_adj, zero_2_cnns, zero_2_cnns,
                                       neighbor_candi);
                Robust_Prune_Build(cnns_id, 0, cur_lock_num, neighbor_candi, zero_layer_adj[i], 1.0);
                Make_Edge_comp(i, 0, 0, zero_layer_locked_num_l, zero_layer_adj, zero_2_cnns, zero_2_cnns, 1.0);
            }

#pragma omp parallel for schedule(dynamic)
            for (CNODEID i = 0; i < ntotal_; i++) {

                CNODEID cnns_id = Get_Cnns_Id_By_Cur_Layer_Id(i, zero_2_cnns);
                size_t cur_compo = Get_Current_Component_By_Cnns_Id(cnns_id, 0);
                enter_pt = sp_l_list[0][cur_compo];
                CDEGREE cur_lock_num = 0;
                maxHeap<CDIST> neighbor_candi;
                Search_Neighbors_Build(cnns_id, enter_pt, 0, zero_layer_adj, zero_2_cnns, zero_2_cnns,
                                       neighbor_candi);
                Robust_Prune_Build(cnns_id, 0, cur_lock_num, neighbor_candi, zero_layer_adj[i], alpha_);
                Make_Edge_comp(i, 0, 0, zero_layer_locked_num_l, zero_layer_adj, zero_2_cnns, zero_2_cnns, alpha_);
            }
            for (auto i = 0; i < ntotal_; ++i) {
                CNODEID cur_cnns_id = i;
                for (auto j = 0; j < static_cast<CDEGREE >(zero_layer_adj[i].size()); ++j) {
                    auto cur_nei = zero_layer_adj[i][j];
                    Return_Adj_Track(i).emplace_back(Return_Maximum_Common_Layer(cur_nei, cur_cnns_id),
                                                     cur_nei);
                }
            }
        }


        void build_one_layer_comp(CCORE cur_core, CLAYER cur_layer) { //vamana method
            omp_set_num_threads(numThreads);
            std::cout << "Layer: " << cur_layer << ", coreness = " << cur_core << " building.\n";
            CNODEID cnt_cur_layer_num = 0;
            std::vector<CNODEID> cur_layer_2_cnns = {};
            std::vector<CNODEID> cnns_2_cur_layer(ntotal_, -1);

            for (CNODEID i = 0; i < ntotal_; i++) {
                if (core_list[i] >= cur_core) {
                    cur_layer_2_cnns.emplace_back(i);
                    cnns_2_cur_layer[i] = cnt_cur_layer_num;
                    ++cnt_cur_layer_num;
                }
            }
            std::cout << "Current layer has " << cnt_cur_layer_num << " nodes!\n";

#pragma omp parallel for schedule(dynamic) num_threads(numThreads)
            for (auto i = 0; i < sp_l_list[cur_core].size(); ++i) {
                auto enter_pt = Find_Enter_Pt(cur_core, i);
                sp_l_list[cur_core][i] = enter_pt;
            }

            auto enter_pt = -1;
            adj_t ind_subgraph(0);
            std::vector<CDEGREE> locked_nums(0);

            Init_m_Core_Graph(ind_subgraph, locked_nums, cur_layer_2_cnns, cur_core, cur_layer);
            std::vector<std::vector<CNODEID>> cur_layer_adj(ind_subgraph);

#pragma omp parallel for num_threads(numThreads) schedule(dynamic)
            for (CNODEID i = 0; i < cnt_cur_layer_num; i++) {
                CDEGREE cur_lock_num = locked_nums[i];
                if (cur_lock_num == max_degree)
                    continue;
                CNODEID cnns_id = Get_Cnns_Id_By_Cur_Layer_Id(i, cur_layer_2_cnns);

                size_t cur_compo = Get_Current_Component_By_Cnns_Id(cnns_id, cur_core);
                enter_pt = sp_l_list[cur_core][cur_compo];

                maxHeap<CDIST> neighbor_candi;
                Search_Neighbors_Build(cnns_id, enter_pt, cur_core, cur_layer_adj, cur_layer_2_cnns, cnns_2_cur_layer,
                                       neighbor_candi);
                Robust_Prune_Build(cnns_id, cur_layer, cur_lock_num, neighbor_candi, cur_layer_adj[i], alpha_);
                Make_Edge_comp(i, cur_core, cur_layer, locked_nums, cur_layer_adj, cur_layer_2_cnns, cnns_2_cur_layer,
                               alpha_);
            }

            for (auto i = 0; i < cur_layer_2_cnns.size(); ++i) {
                CNODEID cur_cnns_id = cur_layer_2_cnns[i];
                for (auto j = locked_nums[i]; j < static_cast<CDEGREE >(cur_layer_adj[i].size()); ++j) {
                    auto cur_nei = cur_layer_adj[i][j];
                    hier_adj[cur_cnns_id].emplace_back(Return_Maximum_Common_Layer(cur_nei, cur_cnns_id),
                                                       cur_nei);
                }
            }
        }

        void NSW_Init_Roreder_Ver(CLAYER cur_layer_num,
                                  CCORE cur_coreness,
                                  std::vector<std::vector<CNODEID>> &cur_layer_adj,
                                  std::vector<CNODEID> &cur_layer_2_cnns,
                                  std::vector<CNODEID> &cnns_2_cur_layer,
                                  CNODEID enterpt_fake) {

#pragma omp parallel for num_threads(numThreads) schedule(dynamic)
            for (CNODEID i = 0; i < cur_layer_adj.size(); i++) {
                CNODEID cnns_id;
                if (cur_coreness == 0) {
                    cnns_id = Get_Cnns_Id_By_Cur_Layer_Id(reordered_2_cnns[i], cur_layer_2_cnns);
                } else if (cur_coreness != 0)
                    cnns_id = cur_layer_2_cnns[i];

                size_t cur_compo = Get_Current_Component_By_Cnns_Id(cnns_id, cur_coreness);
                CNODEID sp = sp_l_list[cur_coreness][cur_compo];

                maxHeap<CDIST> neighbor_candi;
                Search_Neighbors_Build(cnns_id, sp, cur_coreness, cur_layer_adj, cur_layer_2_cnns, cnns_2_cur_layer,
                                       neighbor_candi);
                Robust_Prune_Build(cnns_id, cur_layer_num, 0, neighbor_candi, cur_layer_adj[cnns_2_cur_layer[cnns_id]],
                                   1.0);
                Make_Edge_uncomp(cnns_2_cur_layer[cnns_id], cur_coreness, cur_layer_num, cur_layer_adj,
                                 cur_layer_2_cnns, cnns_2_cur_layer, 1.0);

            }
        }


        void Random_Init(CLAYER cur_layer_num,
                         std::vector<std::vector<CNODEID>> &cur_layer_adj,
                         const std::vector<CNODEID> &cur_layer_2_cnns) {
            srand(unsigned(time(nullptr)));
            if (!cur_layer_adj.empty()) {
                cur_layer_adj.clear();
            }
            for (auto i = 0; i < cur_layer_2_cnns.size(); i++) {
                // 随机生成 当前层 第i个node的邻居，放在 set 里
                std::set<CNODEID> random_neighs;
                do {
                    random_neighs.insert(static_cast<CNODEID>(rand() % cur_layer_2_cnns.size()));
                } while (random_neighs.size() < deg_list[0]);
                std::vector<CNODEID> tmp = {};
                for (auto chosen: random_neighs) {
                    tmp.emplace_back(chosen);
                }
                cur_layer_adj.emplace_back(tmp);
            }
        }

        maxHeap<CDIST> Search_Neighbors_Build(const CNODEID q,// q, sp 都是cnns图id
                                              const CNODEID sp,
                                              const CCORE cur_coreness,
                                              adj_t &cur_layer_adj,
                                              std::vector<CNODEID> &cur_layer_2_cnns,
                                              std::vector<CNODEID> &cnns_2_cur_layer,
                                              maxHeap<CDIST> &neighbor_candi) {
            while (!neighbor_candi.empty()) {
                neighbor_candi.pop();
                std::cout << "neighbor_l not empty";
            }
            // 可用数组

            std::vector<bool> vis(cnns_2_cur_layer.size(), false);
            maxHeap<CDIST> resultset;
            maxHeap<CDIST> expandset;

            expandset.emplace(-Cal_Dist(q, sp), sp);
            CDIST higherBound = -expandset.top().first;
            resultset.emplace(-expandset.top().first, expandset.top().second);
            vis[sp] = true;
            CDIST lowerBound = -expandset.top().first;

            while (!expandset.empty()) {

                auto cur = expandset.top();
                if ((-cur.first) > lowerBound)
                    break;
                expandset.pop();
                CNODEID cur_exp_id = cnns_2_cur_layer[cur.second];

                std::unique_lock<std::mutex> lk(link_list_locks[cur.second]);
                auto link = &cur_layer_adj[cur_exp_id];
                auto linksz = link->size();
                if (linksz > deg_list[cur_coreness]) {
                    throw (std::runtime_error("Search: link size = " + std::to_string(linksz) + "which is > Degree"));
                }
                for (auto i: *link) {
                    if (vis[i]) continue;

                    auto dist = Cal_Dist(q, i);

                    if (resultset.size() < build_queue_length || dist < lowerBound) {
                        expandset.emplace(-dist, i);
                        vis[i] = true;
                        resultset.emplace(dist, i);
                        if (resultset.size() > build_queue_length) resultset.pop();
                        if (!resultset.empty()) lowerBound = resultset.top().first;
                    }
                }
                neighbor_candi.emplace(std::move(cur));
            }
            return neighbor_candi;
        }

// The first term of the expression (4).
        std::unordered_map<CNODEID, CDIST> Gen_hop_dist(CNODEID q, CCORE coreness) {
            std::unordered_map<CNODEID, CDIST> hop_2_dist;
            std::queue<CNODEID> bfs_queue;
            bfs_queue.emplace(q);
            hop_2_dist.emplace(q, 0);

            auto hop_2_sdist1 = [this](int hop) { return gamma * (hop - 1); };
            int hop = 1;
            int cur_expand_cnt = 0;
            int need_expand_cnt = 1;
            int need_next_expand_cnt = 0;
            while (!bfs_queue.empty()) {
                if (cur_expand_cnt++ < need_expand_cnt) {
                    auto cur_expand = bfs_queue.front();
                    for (auto i: cur_graph_ori_adj[cur_expand]) {
                        if (core_list[i] >= coreness) {
                            auto nei_iter = hop_2_dist.find(i);
                            if (nei_iter == hop_2_dist.end()) {
                                hop_2_dist.emplace(i, hop_2_sdist1(hop));
                                need_next_expand_cnt++;
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
            return hop_2_dist;
        }

        std::vector<CNODEID> Retrieve(CNODEID p, float r, CCORE cur_coreness, size_t *time) {
            maxHeap<CDIST> neighbor_candi;

            CNODEID q = -1;
            if (ori_2_cnns_map.count(p)) {
                q = ori_2_cnns_map.find(p)->second;
            } else {
                std::cout << "Current id is not a supported id!\n";
                return {};
            }
            size_t cur_component = 0;
            if (core_list[q] >= cur_coreness){
                cur_component = Get_Current_Component_By_Cnns_Id(q, cur_coreness);
            } else {
                std::cout << p << " maybe a wrong query as its coreness is low, trying to search...\n";
                int tmp = 0;
                for (auto j = 0; j < node_num_per_compo[cur_coreness].size(); ++j){
                       if (node_num_per_compo[cur_coreness][j] > tmp){
                           cur_component = j;
                           tmp = node_num_per_compo[cur_coreness][j];
                       }
                }
            }
            search_queue_length = L_scale * node_num_per_compo[cur_coreness][cur_component] * r;

            auto s = CLOCK;
            std::unordered_map<CNODEID, float> hop_2_dist = Gen_hop_dist(q, cur_coreness);

            auto dist_to_q = [this, q, &hop_2_dist](CNODEID chci_id) {
                return (Cal_Dist(q, chci_id) + hop_2_dist[chci_id]);
            };

            CDEGREE cur_deg = max_degree;
            auto sp_1 = sp_l_list[cur_coreness][cur_component];
            auto sp_2 = q;
            std::vector<bool> vis(ntotal_, false);
            while (!neighbor_candi.empty()) {
                neighbor_candi.pop();
                std::cout << "neighbor_l not empty";
            }

            maxHeap<CDIST> resultset;
            maxHeap<CDIST> expandset;

            expandset.emplace(-dist_to_q(sp_1), sp_1);
            resultset.emplace(dist_to_q(sp_1), sp_1);
            vis[sp_1] = true;

            if (core_list[sp_2] >= cur_coreness){
            expandset.emplace(-dist_to_q(sp_2), sp_2);
            vis[sp_2] = true;
            }
            resultset.emplace(0, sp_2);

            CDIST lowerBound = -expandset.top().first;

            /* ****** Search begin!!! ****** */
            while (!expandset.empty()) {
                auto cur = expandset.top();
                // 退出循环的条件之一
                if ((-cur.first) > lowerBound)
                    break;
                expandset.pop();
                // 进行当前层与cnns大图的节点标号转化
                int deg_cnt = max_degree;
                for (auto ii: hier_adj[cur.second]) {
                    if (deg_cnt <= 0) {
                        break;
                    }
                    if (ii.first >= cur_coreness) {
                        deg_cnt--;
                        auto i = ii.second;
                        if (vis[i]) continue;
                        vis[i] = true;

                        auto dist = dist_to_q(i);

                        if (resultset.size() < search_queue_length || dist < lowerBound) {
                            expandset.emplace(-dist, i);
                            resultset.emplace(dist, i);
                            if (resultset.size() > search_queue_length) resultset.pop();
                            if (!resultset.empty()) lowerBound = resultset.top().first;
                        }
                    }
                }
            }
            auto e = CLOCK;
            *time += TIME_US(s, e);

            // post-processing
            if (resultset.empty())
                return {};
            std::vector<CNODEID> res;
            res.reserve(search_queue_length + 5);
            if (valid_nodes.empty()) {
                for (auto i = 0; i < node_num_per_compo[cur_coreness][cur_component] * r * (1.2 - 1); ++i) {
                    resultset.pop();
                }
                while (!resultset.empty()) {
                    res.emplace_back(resultset.top().second);
                    resultset.pop();
                }
            } else {
                while (!resultset.empty()) {
                    CNODEID cur_cnns_id = resultset.top().second;
                    if (valid_nodes.count(cur_cnns_id)){
                        res.emplace_back(cur_cnns_id);
                }
                    resultset.pop();
                }
                if (res.size() > node_num_per_compo[cur_coreness][cur_component] * r){
                    res.erase(res.begin(),  res.begin() + res.size()- node_num_per_compo[cur_coreness][cur_component] * r);
                }
            }
                std::sort(res.begin(), res.end());
            std::vector<CNODEID > filted = Filt(res, cur_coreness);
            if (std::find(filted.begin(), filted.end(),p) != filted.end()){
                return filted;
            } else {
                return res;
            }
        }
        
        std::vector<CNODEID> Filt(std::vector<CNODEID > &res, CCORE coreness){
            CNODEID nei_num = coreness;
            if (communityBase = truss){
                nei_num = coreness-1;
            }
            std::vector<CNODEID> res_filt;
            auto f = [this,&res, nei_num](CNODEID v){
                auto begin_iter1 = res.begin();
                auto end_iter1 = res.end();
                auto begin_iter2 = cur_graph_ori_adj[v].begin();
                auto end_iter2 = cur_graph_ori_adj[v].end();
                int cnt = nei_num;
                while (begin_iter1 != end_iter1 && begin_iter2 != end_iter2){
                    if (*begin_iter1 == *begin_iter2){
                        cnt--;
                        if (cnt <=0) break;
                        begin_iter1++;
                        begin_iter2++;
                    } else if (*begin_iter1 < *begin_iter2){
                        begin_iter1++;
                    } else {
                        begin_iter2++;
                    }
                }
                if (cnt <= 0) return true;
                else return false;
            };
            for (auto i : res){
                if (f(i)) res_filt.emplace_back(cnns_2_ori[i]);
            }
            return res_filt;
        }

        void Robust_Prune_Build(CNODEID id,
                                CLAYER cur_layer,
                                CDEGREE locked_nums,
                                maxHeap<CDIST> &cand_set,
                                std::vector<CNODEID> &cur_neigh_l,
                                float alp) {
            std::unique_lock<std::mutex> lock(link_list_locks[id]);
            CDEGREE cur_deg = deg_list[cur_layer];
            if (!cur_neigh_l.empty()) {
                for (auto i = cur_neigh_l.size() - 1; i >= locked_nums && (i + 1); --i) {
                    cand_set.emplace(-Cal_Dist(id, cur_neigh_l[i]), cur_neigh_l[i]);
                }
            }
            cur_neigh_l.resize(locked_nums);

            std::vector<CNODEID> cur_neigh_data_l;
            std::vector<CDIST> cur_neigh_dist_l;
            cur_neigh_data_l.reserve(locked_nums);
            for (auto i = 0; i < locked_nums; ++i) {
                cur_neigh_data_l.emplace_back(cur_neigh_l[i]);
                cur_neigh_dist_l.emplace_back(Cal_Dist(id, cur_neigh_l[i]));
            }

            flat_hash_set<CNODEID> visited = {};
            for (auto j: cur_neigh_l) {
                visited.emplace(j);
            }
            maxHeap<CDIST> recycleBin;

            while (!cand_set.empty()) {
                if (cur_neigh_l.size() >= cur_deg)
                    break;
                auto cur = cand_set.top();
                if (cur.second == id || visited.find(cur.second) != visited.end()) {
                    cand_set.pop();
                    continue;
                }
                cand_set.pop();
                visited.emplace(cur.second);
                bool good = true;
                for (auto j = 0; j < cur_neigh_data_l.size(); ++j) {
                    if (cur_neigh_l[j] == cur.second)
                        break;
                    auto dist = Cal_Dist(cur.second, cur_neigh_data_l[j]);
                    if (dist * alp < -cur.first) {
                        good = false;
                        recycleBin.emplace(cur);
                        break;
                    } else {
                        if (!enable_rppp || alp == 1) {
                            continue;
                        }
                        CDIST x = -cur.first / cur_neigh_dist_l[j];
                        if (x > rp_ub_ || x < rp_lb_) {
                            good = false;
                            recycleBin.emplace(cur);
                            break;
                        }
                    }
                }
                if (good) {
                    cur_neigh_l.emplace_back(cur.second);
                    cur_neigh_data_l.emplace_back(cur.second);
                    cur_neigh_dist_l.emplace_back(Cal_Dist(id, cur.second));
                }
            }

            CDEGREE space_left = cur_deg - cur_neigh_l.size();
            for (auto i = 0; i < space_left && !recycleBin.empty(); ++i) {
                cur_neigh_l.emplace_back(recycleBin.top().second);
                recycleBin.pop();
            }
        }

        void Make_Edge_uncomp(CNODEID cur_node,
                              CCORE cur_core,
                              CLAYER cur_layer,
                              adj_t &cur_layer_adj,
                              std::vector<CNODEID> &cur_layer_2_cnns,
                              std::vector<CNODEID> &cnns_2_cur_layer,
                              float alp) {
            CDEGREE cur_deg = deg_list[cur_layer];
            CNODEID cnns_id = cur_layer_2_cnns[cur_node];
            std::unique_lock<std::mutex> lk_cur_node(link_list_locks[cnns_id]);
            auto *link = &cur_layer_adj[cur_node];


            for (auto i = 0; i < link->size(); i++) {
                CNODEID cur_neighbor_id = (*link)[i];
                std::unique_lock<std::mutex> lk(link_list_locks[cur_neighbor_id]);

                auto neighbor_link = &cur_layer_adj[cnns_2_cur_layer[cur_neighbor_id]];
                if (!isDuplicate(cnns_id, neighbor_link)) {
                    if (neighbor_link->size() < cur_deg) {
                        neighbor_link->emplace_back(cnns_id);
                    } else {
                        maxHeap<CDIST> pruneCandi;
                        auto dist = Cal_Dist(cnns_id, cur_neighbor_id);
                        pruneCandi.emplace(-dist, cnns_id);
                        lk.unlock();
                        Robust_Prune_Build(cur_neighbor_id, cur_layer, 0,
                                           pruneCandi,
                                           *neighbor_link,
                                           alp);
                    }
                }
            }
        }


        void Make_Edge_comp(CNODEID cur_node,
                            CCORE cur_core,
                            CLAYER cur_layer,
                            std::vector<CDEGREE> &cur_layer_locked_nums,
                            adj_t &cur_layer_adj,
                            std::vector<CNODEID> &cur_layer_2_cnns,
                            std::vector<CNODEID> &cnns_2_cur_layer,
                            float alp) {
            CDEGREE cur_deg = max_degree;
            CNODEID cnns_id = cur_layer_2_cnns[cur_node];
            auto link = &cur_layer_adj[cur_node];
            for (auto i = 0; i < link->size(); i++) {
                CNODEID cur_neighbor_id = (*link)[i];
                CDEGREE locked_num = cur_layer_locked_nums[cnns_2_cur_layer[cur_neighbor_id]];
                std::unique_lock<std::mutex> lk(link_list_locks[cur_neighbor_id]);
                auto neighbor_link = &cur_layer_adj[cnns_2_cur_layer[cur_neighbor_id]];
                if (!isDuplicate(cnns_id, neighbor_link)) {
                    if (neighbor_link->size() < cur_deg) {
                        neighbor_link->emplace_back(cnns_id);
                    } else {
                        maxHeap<CDIST> pruneCandi;
                        auto dist = Cal_Dist(cnns_id, cur_neighbor_id);
                        pruneCandi.emplace(-dist, cnns_id);
                        lk.unlock();
                        Robust_Prune_Build(cur_neighbor_id, cur_layer, locked_num,
                                           pruneCandi,
                                           *neighbor_link,
                                           alp);
                    }
                }

            }
        }
// todo:
//

        CSTATUS build_all_layer_part() {
            build_1st_layer_part();
            for (auto i = 0; i <= max_coreness; ++i) {
                build_one_layer_part(i, i);
            }
            return 0;
        }

        void build_1st_layer_part() {

            omp_set_num_threads(numThreads);
            real_core_2_layer_map.emplace(0, 0);
            assert(ntotal_ > 0);
            std::vector<std::vector<CNODEID>> cur_adj;

#pragma omp parallel for schedule(dynamic)
            for (auto i = 0; i < sp_l_list[0].size(); ++i) {
                auto enter_pt = Find_Enter_Pt_part(0, i);
                sp_l_list[0][i] = enter_pt;
            }

            auto enter_pt = sp_l_list[0][0];

            std::vector<std::vector<CNODEID>> zero_layer_adj(ntotal_);
            std::vector<CDEGREE> zero_layer_locked_num_l(ntotal_, 0);
            std::vector<CNODEID> zero_2_cnns;
            for (auto i = 0; i < ntotal_; i++) {
                zero_2_cnns.emplace_back(i);
            }
            NSW_Init_Roreder_Ver_part(0, 0, zero_layer_adj, zero_2_cnns, zero_2_cnns);
            CCONTINUOUS cnt = 0;
            std::vector<CNODEID> valid_node_vec(valid_nodes.begin(), valid_nodes.end());
#pragma omp parallel for schedule(dynamic)
            for (CNODEID i = 0; i < valid_node_vec.size(); i++) {
//                CNODEID cnns_id = Get_Cnns_Id_By_Cur_Layer_Id(i, zero_2_cnns);
                CNODEID cnns_id = valid_node_vec[i];
                size_t cur_compo = Get_Current_Component_By_Cnns_Id(cnns_id, 0);
                enter_pt = sp_l_list[0][cur_compo];
                CDEGREE cur_lock_num = 0;
                maxHeap<CDIST> neighbor_candi;
                Search_Neighbors_Build(cnns_id, enter_pt, 0, zero_layer_adj, zero_2_cnns, zero_2_cnns,
                                       neighbor_candi);
                Robust_Prune_Build(cnns_id, 0, cur_lock_num, neighbor_candi, zero_layer_adj[cnns_id], 1.0);
                Make_Edge_comp(i, 0, 0, zero_layer_locked_num_l, zero_layer_adj, zero_2_cnns, zero_2_cnns, 1.0);
            }

#pragma omp parallel for schedule(dynamic)
            for (CNODEID i = 0; i < valid_node_vec.size(); i++) {
                CNODEID cnns_id = valid_node_vec[i];
                size_t cur_compo = Get_Current_Component_By_Cnns_Id(cnns_id, 0);
                enter_pt = sp_l_list[0][cur_compo];
                CDEGREE cur_lock_num = 0;
                maxHeap<CDIST> neighbor_candi;
                Search_Neighbors_Build(cnns_id, enter_pt, 0, zero_layer_adj, zero_2_cnns, zero_2_cnns,
                                       neighbor_candi);
                Robust_Prune_Build(cnns_id, 0, cur_lock_num, neighbor_candi, zero_layer_adj[cnns_id], alpha_);
                Make_Edge_comp(i, 0, 0, zero_layer_locked_num_l, zero_layer_adj, zero_2_cnns, zero_2_cnns, alpha_);
            }
            for (auto i = 0; i < valid_node_vec.size(); ++i) {
                CNODEID cur_cnns_id = valid_node_vec[i];
                for (auto j = 0; j < static_cast<CDEGREE >(zero_layer_adj[i].size()); ++j) {
                    auto cur_nei = zero_layer_adj[i][j];
                    Return_Adj_Track(i).emplace_back(Return_Maximum_Common_Layer(cur_nei, cur_cnns_id),
                                                     cur_nei);
                }
            }
            cnt = 0;
            cnt /= ntotal_;
        }


        void build_one_layer_part(CCORE cur_core, CCORE cur_layer) { //vamana method


            omp_set_num_threads(numThreads);
            std::cout << "Layer: " << cur_layer << ", coreness = " << cur_core << " building.\n";
            CNODEID cnt_cur_layer_num = 0;
            std::vector<CNODEID> cur_layer_2_cnns = {};
            std::vector<CNODEID> cnns_2_cur_layer(ntotal_, -1);

            for (auto i: valid_nodes) {
                if (core_list[i] >= cur_core) {
                    cur_layer_2_cnns.emplace_back(i);
                    cnns_2_cur_layer[i] = cnt_cur_layer_num;
                    ++cnt_cur_layer_num;
                }
            }
            std::cout << "Current layer has " << cnt_cur_layer_num << " nodes!\n";

#pragma omp parallel for schedule(dynamic) num_threads(numThreads)
            for (auto i = 0; i < sp_l_list[cur_core].size(); ++i) {
                auto enter_pt = Find_Enter_Pt_part(cur_core, i);
                sp_l_list[cur_core][i] = enter_pt;
            }

            auto enter_pt = -1;
            adj_t ind_subgraph(0);
            std::vector<CDEGREE> locked_nums(0);

            Init_m_Core_Graph(ind_subgraph, locked_nums, cur_layer_2_cnns, cur_core, cur_layer);
            std::vector<std::vector<CNODEID>> cur_layer_adj(ind_subgraph);

#pragma omp parallel for num_threads(numThreads) schedule(dynamic)
            for (CNODEID i = 0; i < cnt_cur_layer_num; i++) {
                CDEGREE cur_lock_num = locked_nums[i];
                if (cur_lock_num == max_degree)
                    continue;
                CNODEID cnns_id = Get_Cnns_Id_By_Cur_Layer_Id(i, cur_layer_2_cnns);

                size_t cur_compo = Get_Current_Component_By_Cnns_Id(cnns_id, cur_core);
                enter_pt = sp_l_list[cur_core][cur_compo];

                maxHeap<CDIST> neighbor_candi;
                Search_Neighbors_Build(cnns_id, enter_pt, cur_core, cur_layer_adj, cur_layer_2_cnns, cnns_2_cur_layer,
                                       neighbor_candi);
                Robust_Prune_Build(cnns_id, cur_layer, cur_lock_num, neighbor_candi, cur_layer_adj[i], alpha_);
                Make_Edge_comp(i, cur_core, cur_layer, locked_nums, cur_layer_adj, cur_layer_2_cnns, cnns_2_cur_layer,
                               alpha_);
            }

            for (auto i = 0; i < cur_layer_2_cnns.size(); ++i) {
                CNODEID cur_cnns_id = cur_layer_2_cnns[i];
                for (auto j = locked_nums[i]; j < static_cast<CDEGREE >(cur_layer_adj[i].size()); ++j) {
                    auto cur_nei = cur_layer_adj[i][j];
                    hier_adj[cur_cnns_id].emplace_back(Return_Maximum_Common_Layer(cur_nei, cur_cnns_id),
                                                       cur_nei);
                }
            }
        }

        void NSW_Init_Roreder_Ver_part(CLAYER cur_layer_num,
                                       CCORE cur_coreness,
                                       std::vector<std::vector<CNODEID>> &cur_layer_adj,
                                       std::vector<CNODEID> &cur_layer_2_cnns,
                                       std::vector<CNODEID> &cnns_2_cur_layer) {

            std::vector<CNODEID> valid_node_vec(valid_nodes.begin(), valid_nodes.end());
#pragma omp parallel for num_threads(numThreads) schedule(dynamic)
            for (CNODEID i = 0; i < valid_node_vec.size(); i++) {
                auto cnns_id = valid_node_vec[i];
//                    cnns_id = Get_Cnns_Id_By_Cur_Layer_Id(valid_nodes[i], cur_layer_2_cnns);

                size_t cur_compo = Get_Current_Component_By_Cnns_Id(cnns_id, cur_coreness);
                CNODEID sp = sp_l_list[cur_coreness][cur_compo];

                maxHeap<CDIST> neighbor_candi;
                Search_Neighbors_Build(cnns_id, sp, cur_coreness, cur_layer_adj, cur_layer_2_cnns, cnns_2_cur_layer,
                                       neighbor_candi);
                Robust_Prune_Build(cnns_id, cur_layer_num, 0, neighbor_candi,
                                   cur_layer_adj[cnns_2_cur_layer[cnns_id]],
                                   1.0);
                Make_Edge_uncomp(cnns_2_cur_layer[cnns_id], cur_coreness, cur_layer_num, cur_layer_adj,
                                 cur_layer_2_cnns, cnns_2_cur_layer, 1.0);


            }
        }

        CNODEID Find_Enter_Pt_part(CCORE cur_core, size_t cur_compo) {
            std::vector<CCONTINUOUS> centre(numeric_num + text_num, 0);
            int cnt = 0;
            for (auto i : valid_nodes) {
                if (core_list[i] >= cur_core && compo_l[i][cur_core] == cur_compo) {
                    for (auto j = 0; j < numeric_num; ++j) {
                        centre[j] += data_[i].numeric_data[j];
                    }
                    for (auto j = 0; j < text_num; ++j) {
                        centre[j] += data_[i].text_bin[j];
                    }
                }
                cnt++;
            }
            for (auto &i: centre) {
                i /= cnt;
            }
            CCONTINUOUS min_dist = 999;
            CNODEID enter_pt = -1;
            for (auto i : valid_nodes) {
                if (core_list[i] >= cur_core && compo_l[i][cur_core] == cur_compo) {
                    if (ms_.dist_Find_Enter_pt(centre, data_[i]) < min_dist) {
                        min_dist = ms_.dist_Find_Enter_pt(centre, data_[i]);
                        enter_pt = i;
                    }
                }
            }
            return enter_pt;
        }

        void Build_Init_part(const std::string &graph_path,
                             const std::string &vertex_path,
                             const std::string &data_path,
                             const std::string &edge_path,
                             const std::string &valid_node_path,
                             Dataset_Type speci_dataset,
                             __Metric _metric_,
                             StructureMethod sMethod,
                             Cmodel comBase,
                             float alpha,
                             CCONTINUOUS B,
                             CUINT buildQueueLength,
                             CDEGREE maxDegree,
                             CUINT threadNum,
                             bool rppp_enable) {
            specific_dataset = speci_dataset;
            ms_ = HYBSpace<numeric_num, text_num>(speci_dataset, _metric_);
            sMethod_ = sMethod;
            communityBase = comBase;
            ntotal_ = 0;
            alpha_ = alpha;
            build_queue_length = buildQueueLength;
            max_degree = maxDegree;
            numThreads = threadNum;
            metric_ = _metric_;
            beta_ = B;
            enable_rppp = rppp_enable;
            Begin_Build_Graph_Part(graph_path, vertex_path, data_path, edge_path, valid_node_path, speci_dataset,
                                   sMethod);
        }

        void Load_Graph_bin_part(Dataset_Type ds_t, __Metric mtc, Cmodel cmodel, StructureMethod sMeth,
                                 const std::string &cnns_index_input, const std::string &data_path,
                                 const std::string &graph_path,
                                 const std::string &vertex_path, const std::string &edge_path
                            , const std::string &valid_node_path) {
        omp_set_num_threads(60);
            std::ifstream indexIn(cnns_index_input, std::ios::in | std::ios::binary);
            specific_dataset = ds_t;
            communityBase = cmodel;
            load_dataset_topology *ptmp_graph = new load_dataset_topology(graph_path, vertex_path, specific_dataset, edge_path);
            ntotal_ = ptmp_graph->total_node_num;
            cnns_2_ori = std::move(ptmp_graph->cnns_2_ori);
            ori_2_cnns_map = std::move(ptmp_graph->ori_2_cnns);
            ori_graph_adj = std::move(ptmp_graph->res_graph);
            cur_graph_ori_adj = std::move(ptmp_graph->cur_2_cur_adj);
            cur_graph_ori_adj_complete = cur_graph_ori_adj;
            delete ptmp_graph;

            //  Read Valid Node
            std::ifstream valid_in(valid_node_path);
            if (!valid_in)
                throw (std::runtime_error("Not valid valid nodes!!"));
            valid_nodes.clear();
//            std::set<CNODEID > valid_node;
            CNODEID tmp_ori_id;
            while (valid_in >> tmp_ori_id) {
                valid_nodes.emplace(ori_2_cnns_map[tmp_ori_id]);
            }
            // Induced Graph
            std::mutex write_adj_mutex;
#pragma omp parallel for schedule(dynamic)
            for (auto i = 0; i < ntotal_; ++i) {
                if (valid_nodes.count(i)) {
                    std::vector<CNODEID> filtered_nei;
                    std::set_intersection(valid_nodes.begin(), valid_nodes.end(), cur_graph_ori_adj[i].begin(),
                                          cur_graph_ori_adj[i].end(), std::back_inserter(filtered_nei));

                    std::unique_lock<std::mutex> lk1(write_adj_mutex);
                    cur_graph_ori_adj[i] = std::move(filtered_nei);
                } else {
                    std::unique_lock<std::mutex> lk1(write_adj_mutex);
                    cur_graph_ori_adj[i].clear();
                }
            }

            Metric_Interface(mtc);
            sMethod_ = sMeth;

            if (!indexIn) {
                std::string error_msg = "Error opening Index.\n" + cnns_index_input + "\n";
                throw std::runtime_error(error_msg);
            }

            size_t str_size;
            indexIn.read(reinterpret_cast<char *>(&str_size), sizeof(str_size));

            std::string str = ReadString(indexIn, str_size);
            std::cout << "Dataset type: " << str << std::endl;

            indexIn.read(reinterpret_cast<char *>(&max_degree), sizeof(max_degree));
            indexIn.read(reinterpret_cast<char *>(&build_queue_length), sizeof(build_queue_length));
            indexIn.read(reinterpret_cast<char *>(&numThreads), sizeof(numThreads));
            indexIn.read(reinterpret_cast<char *>(&layer_num), sizeof(layer_num));


            std::cout << "Max degree: " << max_degree << std::endl;
            std::cout << "Build queue length: " << build_queue_length << std::endl;
            std::cout << "Number of threads: " << numThreads << std::endl;

            adj_t sp_l_list_bak;
            sp_l_list_bak.resize(layer_num);
            for (CNODEID i = 0; i < layer_num; ++i) {
                CNODEID cur_layer;
                indexIn.read(reinterpret_cast<char *>(&cur_layer), sizeof(cur_layer));
                size_t tmp_size;
                indexIn.read(reinterpret_cast<char *>(&tmp_size), sizeof(tmp_size));
                sp_l_list_bak[cur_layer].resize(tmp_size);
                indexIn.read(reinterpret_cast<char *>(sp_l_list_bak[cur_layer].data()), tmp_size * 4);
            }

            CNODEID idx_ntotal;
            indexIn.read(reinterpret_cast<char *>(&idx_ntotal), sizeof(idx_ntotal));
            if (ntotal_ != idx_ntotal) {
                throw (std::runtime_error("INVALID graph size"));
            }
            core_list.resize(ntotal_);
            data_.resize(ntotal_);

            for (CNODEID i = 0; i < ntotal_; ++i) {
                indexIn.read(reinterpret_cast<char *>(&(core_list[i])), sizeof(core_list[i]));
            }

            Core_Connective_4_Reorder();
            Core_Connective_part();
            Init_Nodes(data_path.c_str());
            sp_l_list = std::move(sp_l_list_bak);
            compo_mutex = std::vector<std::mutex>(ntotal_);
            node_num_per_compo.resize(layer_num);
            for (auto i = 0; i < layer_num; ++i) {
                std::vector<CNODEID> empty(sp_l_list[i].size(), 0);
                node_num_per_compo[i].resize(sp_l_list[i].size());
            }
            for (auto i = 0; i < ntotal_; ++i){
                if (!valid_nodes.count(i))
                    compo_l[i].clear();
            }
            for (auto &i: compo_l) {
                for (auto j = 0; j < i.size(); ++j) {
                    node_num_per_compo[j][i[j]]++;
                }
            }
            hier_adj.resize(ntotal_);
            for (CNODEID i = 0; i < ntotal_; ++i) {
                CNODEID id;
                indexIn.read(reinterpret_cast<char *>(&id), sizeof(id));
                int ori;
                indexIn.read(reinterpret_cast<char *>(&ori), sizeof(ori));
                if (cnns_2_ori[id] != ori) {
                    throw (std::runtime_error("NEQ!!"));
                }
                int out_size;
                indexIn.read(reinterpret_cast<char *>(&out_size), sizeof(out_size));
                for (size_t j = 0; j < out_size; ++j) {
                    char buffer[3];
                    indexIn.read(buffer, 3);
                    int nei = (static_cast<unsigned char>(buffer[0]) << 0) |
                              (static_cast<unsigned char>(buffer[1]) << 8) |
                              (static_cast<unsigned char>(buffer[2]) << 16);
                    hier_adj[id].emplace_back(Return_Maximum_Common_Layer(id, nei), nei);
                }
            }
            link_list_locks = std::vector<std::mutex>(ntotal_);

        }

        void Begin_Build_Graph_Part(const std::string &graph_path,
                                    const std::string &vertex_path,
                                    const std::string &data_path,
                                    const std::string &edge_path,
                                    const std::string &valid_node_path,
                                    Dataset_Type speci_dataset,
                                    StructureMethod structureMethod) {
            specific_dataset = speci_dataset;
            load_dataset_topology *ptmp_graph = new load_dataset_topology(graph_path, vertex_path, specific_dataset, edge_path);
            ntotal_ = ptmp_graph->total_node_num;
            cnns_2_ori = std::move(ptmp_graph->cnns_2_ori);
            ori_2_cnns_map = std::move(ptmp_graph->ori_2_cnns);
            ori_graph_adj = std::move(ptmp_graph->res_graph);
            cur_graph_ori_adj = std::move(ptmp_graph->cur_2_cur_adj);
            cur_graph_ori_adj_complete = cur_graph_ori_adj;
            delete ptmp_graph;

            //  Read Valid Node
            std::ifstream valid_in(valid_node_path);
            if (!valid_in)
                throw (std::runtime_error("Not valid valid nodes!!"));
            valid_nodes.clear();
//            std::set<CNODEID > valid_node;
            CNODEID tmp_ori_id;
            while (valid_in >> tmp_ori_id) {
                valid_nodes.emplace(ori_2_cnns_map[tmp_ori_id]);
            }
            // Induced Graph
            // Deal adjacency cur_graph_ori_adj
            std::cout << "Loading datasets...\n";
            std::mutex write_adj_mutex;
#pragma omp parallel for num_threads(numThreads) schedule(dynamic)
            for (auto i = 0; i < ntotal_; ++i) {
//                CNODEID cur_ori_id = cnns_2_ori[i];
                if (valid_nodes.count(i)) {
                    std::vector<CNODEID> filtered_nei;
                    std::set_intersection(valid_nodes.begin(), valid_nodes.end(), cur_graph_ori_adj[i].begin(),
                                          cur_graph_ori_adj[i].end(), std::back_inserter(filtered_nei));

                    std::unique_lock<std::mutex> lk1(write_adj_mutex);
                    cur_graph_ori_adj[i] = std::move(filtered_nei);
                } else {
                    std::unique_lock<std::mutex> lk1(write_adj_mutex);
                    cur_graph_ori_adj[i].clear();
                }
            }

            ntotal_ = cur_graph_ori_adj.size();
            hier_adj.resize(ntotal_);
            std::cout << "cur_graph_ori_adj size = " << cur_graph_ori_adj.size() << std::endl
                      << "ori_graph_adj size = " << ori_graph_adj.size() << std::endl
                      << "Valid nodes num: " << valid_nodes.size() << std::endl
                      << "ntotal_ = " << ntotal_ << std::endl;
            std::cout << "decomp begin." << std::endl;

            switch (communityBase) {
                case core: {
                    auto *pcur_decomp = new decomposition<CNODEID>(cur_graph_ori_adj, ntotal_);
                    core_list = std::move(pcur_decomp->BZ_Decomp());
                    delete pcur_decomp;
                    break;
                }
                case truss: {
                    auto *pcur_decomp = new truss_decomposition(cur_graph_ori_adj);
                    pcur_decomp->Parallel_K_Truss_Decomp(numThreads);
                    core_list = std::move(pcur_decomp->node_trussness);
                    delete pcur_decomp;
                    break;
                }
            }

            for (auto i: core_list) {
                if (i > layer_num)
                    layer_num = i;
            }

            max_coreness = layer_num;
            layer_num++;
            deg_list = std::vector<CDEGREE>(layer_num, max_degree);
            std::vector<CNODEID> core_l_num(layer_num);
            for (auto i: core_list) {
                core_l_num[i] += 1;
            }
            for (auto i = core_l_num.size() - 1; i > 0; --i) {
                core_l_num[i - 1] += core_l_num[i];
            }

            for (auto i = 0; i < 10; ++i) {
                deg_list[i] = max_degree;
            }

            sp_list.resize(layer_num, -1);

            Core_Connective_4_Reorder();
            Core_Connective_part();
            Init_Nodes(data_path);

            link_list_locks = std::vector<std::mutex>(ntotal_);

            node_num_per_compo.resize(layer_num);
            for (auto i = 0; i < layer_num; ++i) {
                std::vector<CNODEID> empty(sp_l_list[i].size(), 0);
                node_num_per_compo[i].resize(sp_l_list[i].size());
            }
            for (auto &i: compo_l) {
                for (auto j = 0; j < i.size(); ++j) {
                    node_num_per_compo[j][i[j]]++;
                }
            }

            Paper_A = 1.0 - 1.0 / (alpha_ * alpha_);
            if (beta_ == 0) {
                rp_lb_ = 0;
                rp_ub_ = 9999999;
            } else {
                if (beta_ * beta_ - Paper_A < 0) {
                    throw std::runtime_error("Beta is not valid!!!");
                }
                rp_lb_ = beta_ - std::sqrt(beta_ * beta_ - Paper_A);
                rp_ub_ = beta_ + std::sqrt(beta_ * beta_ - Paper_A);
            }
            std::cout << "All information has been loaded successfully.\n";
        }

        std::vector<std::vector<std::vector<CNODEID>>> Core_Connective_part() {
            compo_l.resize(ntotal_);
            for (auto i = 0; i < ntotal_; ++i) {
                compo_l[i].resize(core_list[i] + 1);
            }

            sp_l_list.resize(0);
            std::vector<std::vector<std::vector<CNODEID> > > ori_overall_all_compo;
            std::vector<CNODEID> splist_num(layer_num, 0);

#pragma omp parallel for schedule(dynamic) num_threads(numThreads)
            for (auto i = 0; i < layer_num; ++i) {
                std::vector<bool> vis(ntotal_, false);
                size_t tmp_all_compo_cnt = 0;

                for (auto j: valid_nodes) {
                    if (core_list[j] >= i && !vis[j]) {
                        std::priority_queue<CNODEID, std::vector<CNODEID>> tmp_Heap;
                        tmp_Heap.emplace(j);
                        vis[j] = true;

                        size_t cur_component = tmp_all_compo_cnt++;
                        compo_l[j][i] = cur_component;
                        while (!tmp_Heap.empty()) {
                            auto cur_node = tmp_Heap.top();
                            tmp_Heap.pop();
                            compo_l[cur_node][i] = cur_component;

                            for (auto k: cur_graph_ori_adj[cur_node]) {
                                if (core_list[k] >= i && !vis[k]) {
                                    tmp_Heap.emplace(k);
                                    vis[k] = true;
                                }
                            }
                        }
                    }
                }

#pragma omp critical
                {
                    splist_num[i] = tmp_all_compo_cnt;
                }
                vis.clear();
                vis.resize(ntotal_, false);
            }
            for (auto i: splist_num) {
                std::vector<CNODEID> tmp_sp_l(i, -1);
                sp_l_list.emplace_back(tmp_sp_l);
            }
            for (auto i = 0; i < ntotal_; ++i){
                if (!valid_nodes.count(i)){
                    compo_l[i].clear();
                }
            }
            return ori_overall_all_compo;
        }

// Add node to the current index.
        CSTATUS Batch_Add(const std::string &need_add_path) { // 读 需要添加的顶点id 文件
            std::vector<CNODEID> need_add;
            std::ifstream in(need_add_path);
            CNODEID tmp_ori_id;
            if (in) {
                while (in >> tmp_ori_id) {
                    need_add.emplace_back(tmp_ori_id);
                }
            } else {
                throw std::runtime_error("Not Valid Need Add Path!!!!!!\n");
            }
//#pragma omp parallel for schedule(dynamic) num_threads(numThreads)
                for (auto i = 0; i < need_add.size(); ++i) {
                    auto ori_id = need_add[i];
                Add_Node(ori_id);
            }
            return 0;
        }

// lazy node deletion
        CSTATUS Batch_Del(const std::string &need_del_path) {
            valid_nodes.clear();
            for (auto i = 0; i < core_list.size(); ++i){
                valid_nodes.emplace(i);
            }
            std::ifstream in(need_del_path);
            if (!in){
                throw (std::runtime_error("Not Valid Need Del Path!!!!!!\n" + need_del_path));
            }
            CNODEID tmp_need_del;
//            std::vector<CNODEID> need_del;
            while (in >> tmp_need_del){
                if (ori_2_cnns_map.count(tmp_need_del)){
//                    need_del.emplace_back(ori_2_cnns_map[tmp_need_del]);
                    valid_nodes.erase(ori_2_cnns_map[tmp_need_del]);
                }
            }
            return 0;
        }

// 加入一个新的顶点
// Batch_Add中给出邻居与属性信息
        void
        Add_Node(CNODEID ori_id) {
            CNODEID cnns_id;
            if (ori_2_cnns_map.contains(ori_id)) {
                cnns_id = ori_2_cnns_map[ori_id];
                std::unique_lock<std::mutex> valid_nodes_lk(valid_nodes_mutex);
                valid_nodes.emplace(cnns_id);
                std::vector<CNODEID> valid_neighbor;
                std::set_intersection(cur_graph_ori_adj_complete[cnns_id].begin(),
                                      cur_graph_ori_adj_complete[cnns_id].end(), valid_nodes.begin(), valid_nodes.end(),
                                      std::back_inserter(valid_neighbor));
                valid_nodes_lk.unlock();
                std::unique_lock<std::mutex> adj_lk(adj_mutex);
                cur_graph_ori_adj[cnns_id] = valid_neighbor;
                adj_lk.unlock();
                Structure_data cur_structData{};
                std::unique_lock<std::mutex> struct_data_lk(struct_data_mutex);
                struct_data_[cnns_id] = Structure_data{cnns_id, valid_neighbor, cnns_2_reordered};
                struct_data_lk.unlock();
                CCORE cur_coreness = App_Coreness(cnns_id);
                std::unique_lock<std::mutex> core_lk(core_list_mutex);
                core_list[cnns_id] = cur_coreness;
                core_lk.unlock();
                //  初始化连通分量，用邻居最主要的分量作为自己的分量
                std::unique_lock<std::mutex> compo_lk(compo_mutex[cnns_id]);
                compo_l[cnns_id].resize(cur_coreness + 1);
                for (auto layer = 0; layer <= cur_coreness; ++layer) {
                    int max_num = 0;
                    int cur_compo = -1;
                    for (auto nei: cur_graph_ori_adj[cnns_id]) {
                        if (core_list[nei] >= layer && compo_l[nei].size() > layer) {
                            int nei_compo = compo_l[nei][layer];
                            if (node_num_per_compo[layer][nei_compo] > max_num) {
                                cur_compo = nei_compo;
                                max_num = node_num_per_compo[layer][nei_compo];
                            }
                        }
                    }
                    if (cur_compo == -1){
//                        throw(std::runtime_error("Error in Init component!!!\n"));
                        cur_compo = 0;
                    }
                    compo_l[cnns_id][layer] = cur_compo;
                }
            } else {
                throw (std::runtime_error("ERROR WHILE ADD NODE"));
            }

            // Search in 0 layer
            maxHeap<CDIST> cand_0;
            Search_Add(cnns_id, 0, cand_0);
            std::vector<CNODEID> add_neighbors;
            Robust_Prune_Add(cnns_id, 0, 0, cand_0, add_neighbors, alpha_);
            std::unique_lock<std::mutex> hier_lk(link_list_locks[cnns_id]);
            for (auto new_nei : add_neighbors){
                hier_adj[cnns_id].emplace_back(Return_Maximum_Common_Layer(cnns_id, new_nei), new_nei);
            }
            hier_lk.unlock();
            Make_Edge_Add(cnns_id, 0, 0, add_neighbors, alpha_);
            for (auto i = 1; i <= core_list[cnns_id]; ++i) {
                cand_0 = {};
                if (!Search_Add(cnns_id, i, cand_0)) {
                    int locked_num = 0;
//                    for (auto jj = 0; jj < hier_adj[cnns_id].size(); ++jj){
//                        if (hier_adj[cnns_id][jj].first >= i)
//                            ++locked_num;
//                    }
                    add_neighbors.clear();
                    for (auto &locked_nei: hier_adj[cnns_id]) {
                        if (locked_nei.first >= i){
                            ++locked_num;
                            add_neighbors.emplace_back(locked_nei.second);
                        }
                    }
                    Robust_Prune_Add(cnns_id, i, locked_num, cand_0, add_neighbors, alpha_);
                    hier_lk.lock();
                    for (auto idx = locked_num; idx < add_neighbors.size(); ++idx){
                        hier_adj[cnns_id].emplace_back(Return_Maximum_Common_Layer(cnns_id, add_neighbors[idx]), add_neighbors[idx]);
                    }
                    hier_lk.unlock();
                    Make_Edge_Add(cnns_id, i, locked_num, add_neighbors, alpha_);
                }
            }
        }

        inline void Del_Node(CNODEID del_cnns_id) {
            auto iter = valid_nodes.find(del_cnns_id);
            if (iter != valid_nodes.end()) {
                valid_nodes.erase(iter);
            } else {
                throw (std::runtime_error("Not a valid deletion!!!"));
            }
        }


        bool Search_Add(CNODEID q, CCORE cur_coreness, maxHeap<CDIST> &neighbor_candi) {
            bool could_find = false;
            auto cur_component = Get_Current_Component_By_Cnns_Id(q, cur_coreness);
            if (cur_component> compo_l[cur_coreness].size())
                return false;

            auto s = CLOCK;
//            std::unordered_map<CNODEID, float> hop_2_dist = Gen_hop_dist(q, cur_coreness);
//            auto paper_dist = [this, q, &hop_2_dist](CNODEID cnns_id) {
////                return (Cal_Dist(q, cnns_id) + hop_2_dist[cnns_id]);
//                return (Cal_Dist(q, cnns_id));
//            };
            auto paper_dist = [this, q](CNODEID cnns_id) {
                return (Cal_Dist(q, cnns_id));
            };


            CDEGREE cur_deg = max_degree;
            auto sp_1 = sp_l_list[cur_coreness][cur_component];
            std::vector<bool> vis(ntotal_, false);

            maxHeap<CDIST> resultset;
            maxHeap<CDIST> expandset;

            expandset.emplace(-paper_dist(sp_1), sp_1);
            resultset.emplace(paper_dist(sp_1), sp_1);
            neighbor_candi.emplace(-paper_dist(sp_1), sp_1);
            vis[sp_1] = true;
            CDIST lowerBound = resultset.top().first;

            /* ****** Search begin!!! ****** */
            while (!expandset.empty()) {
                auto cur = expandset.top();
                // 退出循环的条件之一
                if ((-cur.first) > lowerBound)
                    break;
                neighbor_candi.emplace(expandset.top());
                expandset.pop();
                // 进行当前层与cnns大图的节点标号转化
                int deg_cnt = max_degree;
                for (auto ii: hier_adj[cur.second]) {
                    if (deg_cnt <= 0) {
                        break;
                    }
                    if (ii.first >= cur_coreness) {
                        deg_cnt--;
                        auto i = ii.second;
                        if (vis[i]) continue;
                        vis[i] = true;
                        auto dist = paper_dist(i);
                        if (resultset.size() < build_queue_length || dist < lowerBound) {
                            expandset.emplace(-dist, i);
                            resultset.emplace(dist, i);
                            if (resultset.size() > build_queue_length) resultset.pop();
                            if (!resultset.empty()) lowerBound = resultset.top().first;
                        }
                    }
                }
            }
            if (neighbor_candi.top().second == q) {
                could_find = true;
            }

            return could_find;
        }

        void Robust_Prune_Add(CNODEID id,
                              CLAYER cur_layer,
                              CDEGREE locked_nums,
                              maxHeap<CDIST> &cand_set,
                              std::vector<CNODEID> &cur_neigh_l,
                              float alp) {

//            std::unique_lock<std::mutex> lock(link_list_locks[id]);
            CDEGREE cur_deg = max_degree;
            if (!cur_neigh_l.empty()) {
                for (auto i = cur_neigh_l.size() - 1; i >= locked_nums && (i + 1); --i) {
                    cand_set.emplace(-Cal_Dist(id, cur_neigh_l[i]), cur_neigh_l[i]);
                }
            }
            cur_neigh_l.resize(locked_nums);

            std::vector<CNODEID> cur_neigh_data_l;
            std::vector<CDIST> cur_neigh_dist_l;
            cur_neigh_data_l.reserve(locked_nums);
            for (auto i = 0; i < locked_nums; ++i) {
                cur_neigh_data_l.emplace_back(cur_neigh_l[i]);
                cur_neigh_dist_l.emplace_back(Cal_Dist(id, cur_neigh_l[i]));
            }

            flat_hash_set<CNODEID> visited = {};
            for (auto j: cur_neigh_l) {
                visited.emplace(j);
            }
            maxHeap<CDIST> recycleBin;

            while (!cand_set.empty()) {
                if (cur_neigh_l.size() >= cur_deg)
                    break;
                auto cur = cand_set.top();
                if (cur.second == id || visited.find(cur.second) != visited.end()) {
                    cand_set.pop();
                    continue;
                }
                cand_set.pop();
                visited.emplace(cur.second);
                bool good = true;
                for (auto j = 0; j < cur_neigh_data_l.size(); ++j) {
                    if (cur_neigh_l[j] == cur.second)
                        break;
                    auto dist = Cal_Dist(cur.second, cur_neigh_data_l[j]);
                    if (dist * alp < -cur.first) {
                        good = false;
                        recycleBin.emplace(cur);
                        break;
                    } else {
                        if (!enable_rppp || alp == 1) {
                            continue;
                        }
                        CDIST x = -cur.first / cur_neigh_dist_l[j];
                        if (x > rp_ub_ || x < rp_lb_) {
                            good = false;
                            recycleBin.emplace(cur);
                            break;
                        }
                    }
                }
                if (good) {
                    cur_neigh_l.emplace_back(cur.second);
                    cur_neigh_data_l.emplace_back(cur.second);
                    cur_neigh_dist_l.emplace_back(Cal_Dist(id, cur.second));
                }
            }

            CDEGREE space_left = cur_deg - cur_neigh_l.size();
            for (auto i = 0; i < space_left && !recycleBin.empty(); ++i) {
                cur_neigh_l.emplace_back(recycleBin.top().second);
                recycleBin.pop();
            }
        }

        //  使用新加入的顶点替换某些顶底的原有邻居
        void Make_Edge_Add(CNODEID cur_node,
                           CCORE cur_core,
                           CLAYER cur_layer,
                           std::vector<CNODEID> &cur_neighbors,
                           float alp) {
            CDEGREE cur_deg = max_degree;
            CNODEID cnns_id = cur_node;
            for (auto i = 0; i < cur_neighbors.size(); i++) {
                CNODEID cur_neighbor_id = cur_neighbors[i];
                CDEGREE locked_num = 0;
                // nei2: neighbor's neighbor
                std::vector<CNODEID> nei_neighbor;
                nei_neighbor.resize(max_degree);
                int cnt = 0;
                for (auto nei2: hier_adj[cur_neighbor_id]) {
                    if (nei2.first >= cur_core) {
                        nei_neighbor.emplace_back(nei2.second);
                        cnt++;
                        if (cnt >= max_degree) {
                            break;
                        }
                    }
                }
                std::vector<CNODEID> nei_neighbor_bak(nei_neighbor);

                if (!isDuplicate(cnns_id, &nei_neighbor)) {
                    if (nei_neighbor.size() < cur_deg) {
                    } else {
                        maxHeap<CDIST> pruneCandi;
                        auto dist = Cal_Dist(cnns_id, cur_neighbor_id);
                        pruneCandi.emplace(-dist, cnns_id);
//                        lk.unlock();
                        Robust_Prune_Add(cur_neighbor_id, cur_layer, locked_num,
                                         pruneCandi,
                                         nei_neighbor,
                                         alp);
                    }
                } else {
                    continue;
                }
                // 替换了一个邻居
                if (isDuplicate(cnns_id, &nei_neighbor)) {
                    std::sort(nei_neighbor.begin(), nei_neighbor.end());
                    std::sort(nei_neighbor_bak.begin(), nei_neighbor_bak.end());
                    auto iter1 = nei_neighbor.begin();
                    auto iter2 = nei_neighbor_bak.begin();
                    CNODEID diff_ori = 99999999;
                    while (true) {
                        if (*iter1 != *iter2) {
                            diff_ori = *iter1;
                            break;
                        }
                        iter1++;
                        iter2++;
                    }
                    if (diff_ori < ntotal_) {
                        auto cur_mcl = Return_Maximum_Common_Layer(cur_neighbor_id, cur_node);
                        std::pair<CCORE, CNODEID> replace_bak;
                        bool flag = false;
                        std::unique_lock<std::mutex> lk_nei(link_list_locks[cur_neighbor_id]);
                        for (auto &nei2: hier_adj[cur_neighbor_id]) {
                            if (nei2.second == diff_ori) {
                                if (nei2.first <= cur_mcl) {
                                    nei2.second = cur_node;
                                } else {
                                    flag = true;
                                    replace_bak = nei2;
                                    nei2.second = cur_node;
                                    nei2.first = cur_mcl;
                                }
                                break;
                            }
                        }
                        //  插入原来的邻居
                        if (flag) {
                            int cnt = 0;
                            for (auto iter = hier_adj[cur_neighbor_id].begin();
                                 iter != hier_adj[cur_neighbor_id].end(); ++iter) {
                                if (iter->first > cur_mcl) {
                                    cnt++;
                                    if (cnt == max_degree - 1) {
                                        hier_adj[cur_neighbor_id].insert(++iter, replace_bak);
                                        break;
                                    }
                                }
                            }
                        }
                    } else {
                        throw (std::runtime_error("ERROR IN MAKE EDGE!!\n"));
                    }
                }
            }
        }

        //  decide the approximate coreness
        CCORE App_Coreness(CNODEID cnns_id) {
            CCORE res = 0;
            switch (communityBase) {
                case core: {
//                    std::priority_queue<CCORE> neighbor_coreness;
                    std::vector<CCORE> neighbor_coreness;
                    if (cur_graph_ori_adj[cnns_id].size() == 0){
                        break;
                    }
                    for (auto i: cur_graph_ori_adj[cnns_id]) {
                        neighbor_coreness.emplace_back(core_list[i]);
                    }
                    std::sort(neighbor_coreness.begin(), neighbor_coreness.end());
                    CCORE cnt = 0;
                    for (auto i = neighbor_coreness.size() - 1; i >= 0; --i) {
                        if (neighbor_coreness[i] > i) {
                            cnt++;
                        } else {
                            break;
                        }
                    }
                    res = std::max(cnt, 1);
                }
                    break;
                case truss: {
                    res = 2;
                    for (auto p: cur_graph_ori_adj[cnns_id]) {
                        CCORE trussness = 0;
                        CCORE nei_trussness = core_list[p];
                        std::vector<CNODEID >  common_nei;
                        std::set_intersection(cur_graph_ori_adj[cnns_id].begin(), cur_graph_ori_adj[cnns_id].end()
                                    , cur_graph_ori_adj[p].begin(), cur_graph_ori_adj[p].end()
                                    , std::back_inserter(common_nei));
                        int support = common_nei.size();
                        std::vector<CCORE> common_nei_trussness;
                        for (auto commonNei : common_nei){
                            common_nei_trussness.emplace_back(core_list[commonNei]);
                        }
                        std::sort(common_nei_trussness.begin(), common_nei_trussness.end());
                        int cnt = 2;
                        for (auto i = common_nei_trussness.rbegin(); i != common_nei_trussness.rend(); ++i){
                            if (cnt >= *i){
                                trussness = cnt;
                                break;
                            } else {
                                cnt++;
                            }
                        }
                        trussness = std::min(nei_trussness, trussness);
                        res = std::max(trussness, res);
                    }
                }
                break;
            }
            return res;
        }

        inline bool isDuplicate(const CNODEID cur_node, std::vector<CNODEID> *link) {
            for (auto i: *link) {
                if (cur_node == i)
                    return true;
            }
            return false;
        }

        // 用于建图时生成诱导子图

        void Init_m_Core_Graph(adj_t &res_graph,
                               std::vector<CDEGREE> &locked_nums,
                               std::vector<CNODEID> &cur_2_cnns,
                               CCORE m,
                               CLAYER cur_layer) {
            CNODEID cur_total_nodes = cur_2_cnns.size();
            CDEGREE deg_limit = deg_list[cur_layer];
            res_graph.resize(0);
            for (CNODEID i = 0; i < cur_total_nodes; ++i) {
                auto cur_cnns_id = cur_2_cnns[i];
                std::vector<CNODEID> cur_init_neighs(0);
                Read_m_Core_Neighs_Build(cur_cnns_id, cur_layer, deg_limit, cur_init_neighs);
                res_graph.emplace_back(cur_init_neighs);
                locked_nums.emplace_back(cur_init_neighs.size());
            }
        }


        void
        Read_m_Core_Neighs_Build(CNODEID cnns_id, CLAYER cur_layer, CLAYER deg_limit,
                                 std::vector<CNODEID> &res) {
            assert(res.empty());
            CDEGREE cnt = 0;
            for (auto i: hier_adj[cnns_id]) {
                if (i.first >= cur_layer) {
                    res.emplace_back(i.second);
                    ++cnt;
                }
                if (cnt == deg_limit) {
                    break;
                }
            }
        }

        std::vector<std::vector<std::vector<CNODEID>>> Core_Connective() {
            compo_l.resize(ntotal_);
            for (auto i = 0; i < ntotal_; ++i) {
                compo_l[i].resize(core_list[i] + 1);
            }

            sp_l_list.resize(0);
            std::vector<std::vector<std::vector<CNODEID> > > ori_overall_all_compo;
            std::vector<CNODEID> splist_num(layer_num, 0);

#pragma omp parallel for schedule(dynamic) num_threads(numThreads)
            for (auto i = 0; i < layer_num; ++i) {
                std::vector<bool> vis(ntotal_, false);
                size_t tmp_all_compo_cnt = 0;


                for (auto j = 0; j < ntotal_; ++j) {
                    if (core_list[j] >= i && !vis[j]) {
                        std::priority_queue<CNODEID, std::vector<CNODEID>> tmp_Heap;
                        tmp_Heap.emplace(j);
                        vis[j] = true;

                        size_t cur_component = tmp_all_compo_cnt++;
                        compo_l[j][i] = cur_component;
                        while (!tmp_Heap.empty()) {
                            auto cur_node = tmp_Heap.top();
                            tmp_Heap.pop();
                            compo_l[cur_node][i] = cur_component;

                            for (auto k: cur_graph_ori_adj[cur_node]) {
                                if (core_list[k] >= i && !vis[k]) {
                                    tmp_Heap.emplace(k);
                                    vis[k] = true;
                                }
                            }
                        }
                    }
                }

#pragma omp critical
                {
                    splist_num[i] = tmp_all_compo_cnt;
                }
                vis.clear();
                vis.resize(ntotal_, false);
            }
            for (auto i: splist_num) {
                std::vector<CNODEID> tmp_sp_l(i, -1);
                sp_l_list.emplace_back(tmp_sp_l);
            }

            return ori_overall_all_compo;
        }


        inline size_t Get_m_Core_Component(CNODEID i, CCORE cur_coreness) {
            return compo_l[i][cur_coreness];
        }

#define TOSTRING(str) #str

#define OUT(stream, _str)                                                      \
  stream.write(reinterpret_cast<const char *>(&_str), sizeof(_str))

  void Save_Graph_bin(const std::string &cnns_index_output) {
    std::ofstream indexOut(cnns_index_output, std::ios::out | std::ios::binary);
    auto str = Print_Dataset_Type(specific_dataset);
    auto str_size = str.size();
    indexOut.write(reinterpret_cast<const char *>(&str_size), sizeof(str_size));
    indexOut.write(str.c_str(), str_size);
    indexOut.write(reinterpret_cast<const char *>(&max_degree),
                   sizeof(max_degree));
    indexOut.write(reinterpret_cast<const char *>(&build_queue_length),
                   sizeof(build_queue_length));
    indexOut.write(reinterpret_cast<const char *>(&numThreads),
                   sizeof(numThreads));
    indexOut.write(reinterpret_cast<const char *>(&layer_num),
                   sizeof(layer_num));

    for (CNODEID i = 0; i < layer_num; ++i) {
      indexOut.write(reinterpret_cast<const char *>(&i), sizeof(i));
      size_t tmp_size = sp_l_list[i].size();
      OUT(indexOut, tmp_size);
      indexOut.write(reinterpret_cast<const char *>(sp_l_list[i].data()),
                     tmp_size * 4);
    }
    OUT(indexOut, ntotal_);
    for (CNODEID i = 0; i < ntotal_; ++i) {
      OUT(indexOut, core_list[i]);
    }
    for (CNODEID i = 0; i < ntotal_; ++i) {
      OUT(indexOut, i);
      OUT(indexOut, cnns_2_ori[i]);
      int out_size = Return_Adj_Track(i).size();
      OUT(indexOut, out_size);
      for (auto &j : Return_Adj_Track(i)) {
        char buffer[3];
        buffer[0] = static_cast<char>(j.second & 0xFF);
        buffer[1] = static_cast<char>((j.second >> 8) & 0xFF);
        buffer[2] = static_cast<char>((j.second >> 16) & 0xFF);
        indexOut.write(buffer, 3);
      }
    }
  }

  std::string ReadString(std::ifstream &ifs, size_t length) {
    std::vector<char> buffer(length);
    ifs.read(buffer.data(), length);
    return std::string(buffer.data(), length);
  }

  void
  Load_Graph_bin(Dataset_Type ds_t, __Metric mtc, Cmodel cmodel,
                 StructureMethod sMeth, const std::string &cnns_index_input,
                 const std::string &data_path, const std::string &graph_path,
                 const std::string &vertex_path, const std::string &edge_path) {
    std::ifstream indexIn(cnns_index_input, std::ios::in | std::ios::binary);
    specific_dataset = ds_t;
    communityBase = cmodel;
    auto *ptmp_graph = new load_dataset_topology(graph_path.c_str(), vertex_path.c_str(),
                                                 ds_t, edge_path.c_str());
    cnns_2_ori = std::move(ptmp_graph->cnns_2_ori);
    ori_2_cnns_map = std::move(ptmp_graph->ori_2_cnns);
    ori_graph_adj = std::move(ptmp_graph->res_graph);
    cur_graph_ori_adj = std::move(ptmp_graph->cur_2_cur_adj);
    ntotal_ = ptmp_graph->total_node_num;
    delete ptmp_graph;
    Metric_Interface(mtc);
    sMethod_ = sMeth;

    if (!indexIn) {
      std::string error_msg =
          "Error opening Index.\n" + cnns_index_input + "\n";
      throw std::runtime_error(error_msg);
    }

    size_t str_size;
    indexIn.read(reinterpret_cast<char *>(&str_size), sizeof(str_size));

    std::string str = ReadString(indexIn, str_size);
    std::cout << "Dataset type: " << str << std::endl;

    indexIn.read(reinterpret_cast<char *>(&max_degree), sizeof(max_degree));
    indexIn.read(reinterpret_cast<char *>(&build_queue_length),
                 sizeof(build_queue_length));
    indexIn.read(reinterpret_cast<char *>(&numThreads), sizeof(numThreads));
    indexIn.read(reinterpret_cast<char *>(&layer_num), sizeof(layer_num));

    std::cout << "Max degree: " << max_degree << std::endl;
    std::cout << "Build queue length: " << build_queue_length << std::endl;
    std::cout << "Number of threads: " << numThreads << std::endl;

    adj_t sp_l_list_bak;
    sp_l_list_bak.resize(layer_num);
    for (CNODEID i = 0; i < layer_num; ++i) {
      CNODEID cur_layer;
      indexIn.read(reinterpret_cast<char *>(&cur_layer), sizeof(cur_layer));
      size_t tmp_size;
      indexIn.read(reinterpret_cast<char *>(&tmp_size), sizeof(tmp_size));
      sp_l_list_bak[cur_layer].resize(tmp_size);
      indexIn.read(reinterpret_cast<char *>(sp_l_list_bak[cur_layer].data()),
                   tmp_size * 4);
    }

    CNODEID idx_ntotal;
    indexIn.read(reinterpret_cast<char *>(&idx_ntotal), sizeof(idx_ntotal));
    if (ntotal_ != idx_ntotal) {
      throw(std::runtime_error("INVALID graph size"));
    }
    core_list.resize(ntotal_);
    data_.resize(ntotal_);

    for (CNODEID i = 0; i < ntotal_; ++i) {
      indexIn.read(reinterpret_cast<char *>(&(core_list[i])),
                   sizeof(core_list[i]));
    }

    Core_Connective_4_Reorder();
    Core_Connective();
    Init_Nodes(data_path.c_str());
    sp_l_list = std::move(sp_l_list_bak);
    node_num_per_compo.resize(layer_num);
    for (auto i = 0; i < layer_num; ++i) {
      std::vector<CNODEID> empty(sp_l_list[i].size(), 0);
      node_num_per_compo[i].resize(sp_l_list[i].size());
    }
    for (auto &i : compo_l) {
      for (auto j = 0; j < i.size(); ++j) {
        node_num_per_compo[j][i[j]]++;
      }
    }
    hier_adj.resize(ntotal_);
    for (CNODEID i = 0; i < ntotal_; ++i) {
      CNODEID id;
      indexIn.read(reinterpret_cast<char *>(&id), sizeof(id));
      int ori;
      indexIn.read(reinterpret_cast<char *>(&ori), sizeof(ori));
      if (cnns_2_ori[id] != ori) {
        throw(std::runtime_error("NEQ!!"));
      }
      int out_size;
      indexIn.read(reinterpret_cast<char *>(&out_size), sizeof(out_size));
      for (size_t j = 0; j < out_size; ++j) {
        char buffer[3];
        indexIn.read(buffer, 3);
        int nei = (static_cast<unsigned char>(buffer[0]) << 0) |
                  (static_cast<unsigned char>(buffer[1]) << 8) |
                  (static_cast<unsigned char>(buffer[2]) << 16);
        hier_adj[id].emplace_back(Return_Maximum_Common_Layer(id, nei), nei);
      }
    }
  }
    
  inline CNODEID
  Get_Cnns_Id_By_Cur_Layer_Id(const CNODEID id,
                              const std::vector<CNODEID> &cur_layer_2_cnns) {
    return cur_layer_2_cnns[id];
  }

  inline size_t Get_Current_Component_By_Cnns_Id(CNODEID cur_cnns_id,
                                                 CCORE cur_coreness) {
    return compo_l[cur_cnns_id][cur_coreness];
  }

  inline CLAYER Return_Maximum_Common_Layer(CNODEID A, CNODEID B) {
    CLAYER res = 0;
    auto &A_compo = compo_l[A];
    auto &B_compo = compo_l[B];
    CLAYER min = std::min(core_list[A], core_list[B]);
    for (res = min; res > 0; --res) {
      if (A_compo[res] == B_compo[res]) {
        break;
      }
    }
    return res;
  }

  inline std::vector<std::pair<CLAYER, CNODEID>> &
  Return_Adj_Track(CNODEID cnns_id) {
    return hier_adj[cnns_id];
  }

  inline CDIST Cal_Dist(CNODEID a, CNODEID b) {
    return ms_.full_dist(data_[a], data_[b], &struct_data_[a],
                         &struct_data_[b]);
  }

};
}

#endif
