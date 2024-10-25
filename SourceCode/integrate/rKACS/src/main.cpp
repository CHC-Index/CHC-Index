int run_wo = 1;
#include "Def.h"
#include "MaxMinWeightTruss.h"
#include "SFormat.h"
#include <sstream>
#include "chci.h"

#if defined(_WIN64)
#include "Snap.cpp"
#endif

#define CLOCK std::chrono::high_resolution_clock::now()
#define TIME_MS(s, e)                                                          \
  std::chrono::duration_cast<std::chrono::milliseconds>(e - s).count()
#define TIME_US(s, e)                                                          \
  std::chrono::duration_cast<std::chrono::microseconds>(e - s).count()
int DEFAULT_K = 0;
int DEFAULT_R = 0;

std::vector<std::pair<int, std::vector<int>>>
Read_CNNS_Res(std::string CNNS_Res);

void RQuery_Time(MaxMinWeightTruss &maxWeightTruss, string dataSet) {
    const string type = "r", type2 = "time";
    cout << "RQuery_Time() start..." << endl;

    ifstream queryFile(SFormat(QUERY_FILE, dataSet, type));
    ofstream avgTimeFile(SFormat(AVG_RESULT_FILE, dataSet, type2, type));

    string rValueStr;
    vector<int> rValues;
    queryFile >> rValueStr;
    SplitInt(rValueStr, rValues, ",");

    string methodValueStr;
    vector<int> methodValues;
    queryFile >> methodValueStr;
    SplitInt(methodValueStr, methodValues, ",");

    int queryCnt;
    queryFile >> queryCnt;
    vector<vector<int>> queryAttrs;

    for (int i = 0; i < queryCnt; i++) {
        string queryAttrStr;
        vector<int> queryAttr;
        queryFile >> queryAttrStr;
        SplitInt(queryAttrStr, queryAttr, ",");
        queryAttrs.push_back(queryAttr);
    }

    for (int m : methodValues) {
        flat_hash_set<int> failedIndexes;
        for (int r : rValues) {
            avgTimeFile << "r = " << r << " :" << endl;
            cout << "r = " << r << " :" << endl;

            ofstream optFile(
                    SFormat(RESULT_FILE, dataSet, type2, type, r, METHOD_NAMES[m - 1]));
            double avgTime = 0;
            int failedCnt = 0, successCnt = 0;
            for (int i = 0; i < queryCnt; i++) {
                vector<int> queryAttr = queryAttrs[i];
                Time time =
                        failedIndexes.find(i) != failedIndexes.end()
                        ? -1
                        : maxWeightTruss.Query_TimeWrapper(m, DEFAULT_K, r, queryAttr);
                if (time == -1) {
                    failedCnt++;
                    failedIndexes.emplace(i);
                    time = TIME_LIMIT;
                }
                successCnt++;
                optFile << SFormat("time = {0}s", time) << endl;
                cout << SFormat("time = {0}s", time) << endl;
                avgTime += time;
                if (failedCnt >= queryCnt) {
                    break;
                }
            }

            avgTime = avgTime / successCnt;
            avgTimeFile << SFormat("{0}, avg time = {1}s", METHOD_NAMES[m - 1],
                                   avgTime)
                        << endl;
            cout << SFormat("{0}, avg time = {1}s", METHOD_NAMES[m - 1], avgTime)
                 << endl;
        }
        avgTimeFile << "————————————" << endl;
        cout << "————————————" << endl;
    }
    cout << "RQuery_Time() finish..." << endl;
}

void KQuery_Time(MaxMinWeightTruss &maxWeightTruss, string dataSet) {
    const string type = "k", type2 = "time";
    cout << "KQuery_Time() start..." << endl;

    ifstream queryFile(SFormat(QUERY_FILE, dataSet, type));
    ofstream avgTimeFile(SFormat(AVG_RESULT_FILE, dataSet, type2, type));

    string kValueStr;
    vector<int> kValues;
    queryFile >> kValueStr;
    SplitInt(kValueStr, kValues, ",");

    string methodValueStr;
    vector<int> methodValues;
    queryFile >> methodValueStr;
    SplitInt(methodValueStr, methodValues, ",");

    int queryCnt;
    queryFile >> queryCnt;
    vector<vector<int>> queryAttrs;

    for (int i = 0; i < queryCnt; i++) {
        string queryAttrStr;
        vector<int> queryAttr;
        queryFile >> queryAttrStr;
        SplitInt(queryAttrStr, queryAttr, ",");
        queryAttrs.push_back(queryAttr);
    }

    for (int m : methodValues) {
        for (int k : kValues) {
            avgTimeFile << "k = " << k << " :" << endl;
            cout << "k = " << k << " :" << endl;

            ofstream optFile(
                    SFormat(RESULT_FILE, dataSet, type2, type, k, METHOD_NAMES[m - 1]));
            double avgTime = 0;
            int successCnt = 0;
            for (int i = 0; i < queryCnt; i++) {
                vector<int> queryAttr = queryAttrs[i];
                Time time =
                        maxWeightTruss.Query_TimeWrapper(m, k, DEFAULT_R, queryAttr);
                if (time == -1) {
                    time = TIME_LIMIT;
                }
                successCnt++;
                optFile << SFormat("time = {0}s", time) << endl;
                cout << SFormat("time = {0}s", time) << endl;
                avgTime += time;
            }

            avgTime = avgTime / successCnt;
            avgTimeFile << SFormat("{0}, avg time = {1}s", METHOD_NAMES[m - 1],
                                   avgTime)
                        << endl;
            cout << SFormat("{0}, avg time = {1}s", METHOD_NAMES[m - 1], avgTime)
                 << endl;
        }
        avgTimeFile << "————————————" << endl;
        cout << "————————————" << endl;
    }
    cout << "KQuery_Time() finish..." << endl;
}

void AttrCntQuery_Time(MaxMinWeightTruss &maxWeightTruss, string dataSet) {
    // varying the size of |queryAttrs|, from {1, 3, 5, 7, 9, 11, 13}
    const string type = "attrCnt", type2 = "time";
    cout << "AttrCntQuery_Time() start..." << endl;
    ifstream queryFile(SFormat(QUERY_FILE, dataSet, type));
    ofstream avgTimeFile(SFormat(AVG_RESULT_FILE, dataSet, type2, type));

    string attrCntStr;
    vector<int> attrCnts;
    queryFile >> attrCntStr;
    SplitInt(attrCntStr, attrCnts, ",");

    string methodValueStr;
    vector<int> methodValues;
    queryFile >> methodValueStr;
    SplitInt(methodValueStr, methodValues, ",");

    int queryCnt;
    queryFile >> queryCnt;
    vector<vector<int>> queryAttrs;

    for (int i = 0; i < queryCnt; i++) {
        string queryAttrStr;
        vector<int> queryAttr;
        queryFile >> queryAttrStr;
        SplitInt(queryAttrStr, queryAttr, ",");
        queryAttrs.push_back(queryAttr);
    }

    for (int m : methodValues) {
        for (int attrCnt : attrCnts) {
            avgTimeFile << "attrCnt = " << attrCnt << " :" << endl;

            ofstream optFile(SFormat(RESULT_FILE, dataSet, type2, type, attrCnt,
                                     METHOD_NAMES[m - 1]));
            double avgTime = 0;
            int successCnt = 0;
            for (int i = 0; i < queryCnt; i++) {
                vector<int> attrs = queryAttrs[i], queryAttr;
                for (int j = 0; j < attrCnt; j++) {
                    queryAttr.push_back(attrs[j]);
                }
                Time time = maxWeightTruss.Query_TimeWrapper(m, DEFAULT_K, DEFAULT_R,
                                                             queryAttr);
                if (time == -1) {
                    time = TIME_LIMIT;
                }
                successCnt++;
                optFile << SFormat("time = {0}s", time) << endl;
                avgTime += time;
            }

            avgTime = avgTime / successCnt;
            avgTimeFile << SFormat("{0}, avg time = {1}s", METHOD_NAMES[m - 1],
                                   avgTime)
                        << endl;
        }
        avgTimeFile << "————————————" << endl;
    }
    cout << "AttrCntQuery_Time() finish..." << endl;
}

void Query_Quality(MaxMinWeightTruss &maxWeightTruss, string dataSet) {
    const string type = "quality";
    cout << "Query_Quality() start..." << endl;

    ifstream queryFile(SFormat(QUERY_FILE, dataSet, type));

    int queryCnt;
    queryFile >> queryCnt;
    vector<vector<int>> queryAttrs;

    for (int i = 0; i < queryCnt; i++) {
        string queryAttrStr;
        vector<int> queryAttr;
        queryFile >> queryAttrStr;
        SplitInt(queryAttrStr, queryAttr, ",");
        queryAttrs.push_back(queryAttr);
    }

    string methods[3] = {"kc", "kac", "vac"};

    for (int m = 0; m < 3; m++) {
        ofstream resultFile(SFormat(QUALITY_RESULT_FILE, dataSet, methods[m]));
        for (int i = 0; i < queryCnt; i++) {
            vector<int> queryAttr_ = queryAttrs[i];
            ResultMap results;
            if (m == 2) {
                results = maxWeightTruss.VACQuery(DEFAULT_K, queryAttr_);
            } else if (m == 1) {
                results =
                        maxWeightTruss.rKACS_Incremental(DEFAULT_K, DEFAULT_R, queryAttr_);
            } else {
                results = maxWeightTruss.KCQuery(DEFAULT_K, queryAttr_);
            }
            string str = "";
            if (!results.empty()) {
                auto &resultComs = results.begin()->second;
                Clique com = *resultComs.begin();
                for (auto node : com) {
                    str.append(to_string(node) + ",");
                }
                str = str.substr(0, str.length() - 1);
            }
            resultFile << str << endl;
        }
    }

    cout << "Query_Quality() finish..." << endl;
}

void Query_Quality_CNNS(MaxMinWeightTruss &maxWeightTruss,
                        const std::string &CNNS_Res,
                        const std::string &Log_with,
                        const std::string &Log_wo) {
    const string type = "quality";
    cout << "Query_Quality() start..." << endl;

    std::ofstream out_with(Log_with);
    std::ofstream out_wo;
    if (run_wo){
    out_wo.open(Log_wo);
    }

    auto CNNS_Res_Set = Read_CNNS_Res(CNNS_Res);
    //    vector<vector<int>> cnns_edge_set;
    //    for (auto i : CNNS_Res_Set){
    //        vector<pair<int,int>> CNNS_edges;
    //        if (maxWeightTruss.G.)
    //        for (int i = k; i < trussValueCnt; i++)
    //        {
    //            for (auto& edge : trussnessToEdges[i])
    //            {
    //                int u = edge.first, v = edge.second;
    //                if (std::find(CNNS_Res.begin(), CNNS_Res.end(), u) !=
    //                CNNS_Res.end() && std::find(CNNS_Res.begin(),
    //                CNNS_Res.end(), v) != CNNS_Res.end())
    //                    maxKTruss->AddEdgeUnchecked(u, v);
    //                //  是否要加入整个truss的边？
    //            }
  //        }
  //    }
  int queryCnt = CNNS_Res_Set.size();
  //    queryFile >> queryCnt;
  vector<vector<int>> queryAttrs;
  for (int i = 0; i < CNNS_Res_Set.size(); i++) {
    //        string queryAttrStr;
    vector<int> queryAttr;
    //        queryFile >> queryAttrStr;
    //        SplitInt(queryAttrStr, queryAttr, ",");
    //        queryAttrs.push_back(queryAttr);
    // cerr << maxWeightTruss.G->IsNode()
    queryAttrs.push_back(
        maxWeightTruss.attrsOfNode.find(CNNS_Res_Set[i].first)->second);
  }

  //    for (int i = 0; i < queryCnt; i++)
  //    {
  //        string queryAttrStr;
  //        vector<int> queryAttr;
  //        queryFile >> queryAttrStr;
  //        SplitInt(queryAttrStr, queryAttr, ",");
  //        queryAttrs.push_back(queryAttr);
  //    }

  string methods[2] = {"withCNNS", "wo"};
  // string methods[2] = {"wo", "withCNNS"};
  long long wo_time = 0;
  long long with_time = 0;
  float wo_score = 0;
  float with_score = 0;
  int fail_num = 0;

  //    for (int m = 0; m < 2; m++)
  //    {
  //        ofstream resultFile(SFormat(QUALITY_RESULT_FILE, dataSet,
  //        methods[m]));
  for (int i = 0; i < queryCnt; i++) {
    for (int m = 0; m < run_wo + 1; m++) {
      vector<int> queryAttr_ = queryAttrs[i];
      for (auto qq : queryAttr_) {
        std::cerr << qq << " ";
      }
      ResultMap results;
      if (m == 1) {
        auto wo_begin = CLOCK;
        results =
            maxWeightTruss.rKACS_Incremental(DEFAULT_K, DEFAULT_R, queryAttr_);
        auto wo_end = CLOCK;
        if (results.empty()) {
          cerr << "CNNS ID: " << CNNS_Res_Set[i].first << " ERROR!!!!!\n";
          break;
        }
        WeightEdgeMap tmp;
        tmp.clear();
        Clique com = *(results.begin()->second).begin();
        maxWeightTruss.ComputeWeight(com, tmp, false);
        float cur_score = tmp.rbegin()->first.numerator * 1.0 /
                          tmp.rbegin()->first.denominator;
        wo_score += cur_score;
        wo_time += TIME_US(wo_begin, wo_end);

        out_wo << CNNS_Res_Set[i].first << std::endl;
        for (auto i : com) {
          out_wo << i << " ";
        }
        out_wo << std::endl;
        out_wo << "score: " << cur_score << std::endl;
        out_wo << "runtime: " << TIME_US(wo_begin, wo_end) << " us"
               << std::endl;

        cout << "wo_query time " << TIME_US(wo_begin, wo_end) << endl;
        cout << "wo_query score " << cur_score << endl;
      } else {
        auto with_begin = CLOCK;
        results = maxWeightTruss.rKACS_Incremental_CNNS(
            DEFAULT_K, DEFAULT_R, queryAttr_, CNNS_Res_Set[i].second);
        auto with_end = CLOCK;

        if (results.empty()) {
          fail_num++;
          std::cerr << "Failed " + std::to_string(CNNS_Res_Set[i].first)
                    << std::endl;
          continue;
        }
        WeightEdgeMap tmp;
        tmp.clear();
        Clique com = *(results.begin()->second).begin();
        maxWeightTruss.ComputeWeight(com, tmp, false);
        float cur_score = tmp.rbegin()->first.numerator * 1.0 /
                          tmp.rbegin()->first.denominator;
        with_time += TIME_US(with_begin, with_end);
        with_score += cur_score;

        out_with << CNNS_Res_Set[i].first << std::endl;
        for (auto i : com) {
          out_with << i << " ";
        }
        out_with << std::endl;
        out_with << "score: " << cur_score << std::endl;
        out_with << "runtime: " << TIME_US(with_begin, with_end) << " us"
                 << std::endl;

        cout << "with_query time " << TIME_US(with_begin, with_end) << endl;
        cout << "with_query score " << cur_score << endl;
      }
      string str = "";
      if (!results.empty()) {
        auto &resultComs = results.begin()->second;
        Clique com = *resultComs.begin();
        for (auto node : com) {
          str.append(to_string(node) + ",");
        }
        str = str.substr(0, str.length() - 1);
      }
      //            resultFile << str << endl;
      cout << methods[m] << str << endl;
    }
  }

  wo_time /= CNNS_Res_Set.size();
  with_time /= CNNS_Res_Set.size();
  wo_score /= CNNS_Res_Set.size();
  with_score /= CNNS_Res_Set.size();
  cout << "wo time: " << wo_time << endl << "with time : " << with_time << endl;
  cout << "wo score: " << wo_score << endl
       << "with score : " << with_score << endl;
  cout << "Query_Quality() finish..." << endl;
  cout << "Fail num = " << fail_num << std::endl;
}

std::vector<std::pair<int, std::vector<int>>>
Read_CNNS_Res(std::string CNNS_Res) {
  typedef int CNODEID;
  std::ifstream in(CNNS_Res);
  if (!in.is_open()) {
    return {{}};
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
    std::set<CNODEID> tmp_res;
    in_s >> tmp_id;
    std::getline(in, tmpstr);
    in_s.clear();
    in_s.str("");
    in_s.str(tmpstr);
    CNODEID cur_res_nei = -1;
    while (in_s >> cur_res_nei) {
      //                    tmp_res.emplace_back(ori_2_cnns_map.find(cur_res_nei)->second);
      tmp_res.emplace(cur_res_nei);
    }
    std::vector<CNODEID> tmp_res_vec(tmp_res.begin(), tmp_res.end());
    //  ori_id -> ori_id
    query_with_cnnsRes.emplace_back(tmp_id, tmp_res_vec);
    std::getline(in, tmpstr);
  }

  //    std::set<std::pair<CNODEID, std::vector<CNODEID>>> queryS;
  //    for (auto i: query_with_cnnsRes){
  //        queryS.emplace(i.first, i.second);
  //    }
  return query_with_cnnsRes;
}

template <class PGraph>
PGraph LoadEdgeList_CHCI(vector<vector<int>> &cur_2_cur_adj) {
  PGraph Graph = PGraph::TObj::New();
  for (auto i = 0; i < cur_2_cur_adj.size(); ++i) {
    if (!Graph->IsNode(i)) {
      Graph->AddNode(i);
    };
    for (auto j : cur_2_cur_adj[i]) {
      if (j > i) {
        if (!Graph->IsNode(j)) {
          Graph->AddNode(j);
        }
        Graph->AddEdge(i, j);
      }
    }
  }
  Graph->Defrag();
  return Graph;
}
using namespace chci;
void Query_CHCI(int argc, char **argv) {
  //    string dataSet = string(argv[1]);
  int cnt = 1;
  std::string dataset_type = argv[cnt++];
  string graph_path = argv[cnt++];
  string vertex_path = argv[cnt++];
  string edge_path = argv[cnt++];
  string feat_file = string(argv[cnt++]);
  string valid_node_file = string(argv[cnt++]);

  string trussness_file = string(argv[cnt++]);
  string similarity_file = string(argv[cnt++]);

  DEFAULT_K = atoi(argv[cnt++]);
  DEFAULT_R = atoi(argv[cnt++]);
  string CNNS_Res = argv[cnt++];
  string with_Log = argv[cnt++];
  string wo_Log = argv[cnt++];
  run_wo = atoi(argv[cnt++]);

#define SWITCH_DSTYPE(in, str)                                                 \
  if (in == TOSTRING(str))                                                     \
  specific_dataset = str
  Dataset_Type specific_dataset = Facebook;
  SWITCH_DSTYPE(dataset_type, DBLP);
  SWITCH_DSTYPE(dataset_type, IMDB_Person);
  SWITCH_DSTYPE(dataset_type, IMDB_Movie);
  SWITCH_DSTYPE(dataset_type, Twitch);
  SWITCH_DSTYPE(dataset_type, LiveJournal);
  SWITCH_DSTYPE(dataset_type, Facebook);
  SWITCH_DSTYPE(dataset_type, FB0);

  CHCIndex *topo = new chci::chci_graph<1, 1>();
  topo->Load_Graph_Topology(graph_path, vertex_path, edge_path, valid_node_file,
                            specific_dataset);

  MaxMinWeightTruss maxWeightTruss;
  maxWeightTruss.G = LoadEdgeList_CHCI<PUNGraph>(topo->cur_graph_ori_adj);
  cout << "Load graph complete..." << endl;
  if (maxWeightTruss.LoadTrussness(trussness_file.c_str()) == 0) {
    maxWeightTruss.LoadTrussness(trussness_file.c_str());
  }

  cout << "Load trussness complete..." << endl;
  maxWeightTruss.LoadVertexAttribute_CHCI(feat_file, topo->ori_2_cnns_map);
  cout << "Load attributes complete..." << endl;
  if (maxWeightTruss.LoadSimilarity(similarity_file.c_str()) == 0) {
    maxWeightTruss.LoadSimilarity(similarity_file.c_str());
  }
  cout << "Load similarity complete..." << endl;

  auto wo_begin = CLOCK;
  //        Query_Quality(maxWeightTruss, dataSet);
  auto wo_end = CLOCK;
  Query_Quality_CNNS(maxWeightTruss, CNNS_Res, with_Log, wo_Log);
  auto with_end = CLOCK;
  auto wo_time = TIME_MS(wo_end, wo_begin);
  auto with_time = TIME_MS(with_end, wo_end);
  //        std::cout << "wo_time: " << wo_time << endl << "with time: " <<
  //        with_time;
}

void Query(int argc, char **argv) {
  string dataSet = string(argv[1]);
  string type = string(argv[2]);
  string queries = string(argv[3]);
  DEFAULT_K = atoi(argv[4]);
  DEFAULT_R = atoi(argv[5]);
  string CNNS_Res = argv[6];
  string with_Log = argv[7];
  string wo_Log = argv[8];
  run_wo = atoi(argv[9]);

  std::cerr << SFormat(GRAPH_FILE, dataSet);

  MaxMinWeightTruss maxWeightTruss;
  maxWeightTruss.G =
      TSnap::LoadEdgeList<PUNGraph>(SFormat(GRAPH_FILE, dataSet).c_str());
  cout << "Load graph complete..." << endl;
  if (maxWeightTruss.LoadTrussness(SFormat(TRUSSNESS_FILE, dataSet).c_str()) ==
      0) {
    maxWeightTruss.LoadTrussness(SFormat(TRUSSNESS_FILE, dataSet).c_str());
  }
  cout << "Load trussness complete..." << endl;
  maxWeightTruss.LoadVertexAttribute(SFormat(ATTRIBUTE_FILE, dataSet).c_str());
  cout << "Load attributes complete..." << endl;
  if (maxWeightTruss.LoadSimilarity(
          SFormat(SIMILARITY_FILE, dataSet).c_str()) == 0) {
    maxWeightTruss.LoadSimilarity(SFormat(SIMILARITY_FILE, dataSet).c_str());
  }
  cout << "Load similarity complete..." << endl;

  if (type == "t") // time, ignore the output communities
  {
    for (int i = 0; i < queries.size(); i++) {
      int indicator = queries[i];
      if (indicator == 'r') {
        RQuery_Time(maxWeightTruss, dataSet);
      } else if (indicator == 'k') {
        KQuery_Time(maxWeightTruss, dataSet);
      } else if (indicator == 'c') {
        AttrCntQuery_Time(maxWeightTruss, dataSet);
      }
    }
  } else if (type == "q") // quality, save the output communities to file
  {

    auto wo_begin = CLOCK;
    //        Query_Quality(maxWeightTruss, dataSet);
    auto wo_end = CLOCK;
    Query_Quality_CNNS(maxWeightTruss, CNNS_Res, with_Log, wo_Log);
    auto with_end = CLOCK;
    auto wo_time = TIME_MS(wo_end, wo_begin);
    auto with_time = TIME_MS(with_end, wo_end);
    //        std::cout << "wo_time: " << wo_time << endl << "with time: " <<
    //        with_time;
  }
}

int main(int argc, char **argv) { Query_CHCI(argc, argv); }
