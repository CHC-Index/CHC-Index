#include "../include/chci.h"
#define SWITCH_DSTYPE(in, str)                                                 \
  if (in == TOSTRING(str))                                                     \
  Dataset_DT = chci::str

using namespace chci;
using namespace std;
int main(int argc, char **argv) {
  int cnt = 1;
  std::string dataset_type = argv[cnt++];
  string index_p = argv[cnt++];
  string graph_path = argv[cnt++];
  string vertex_path = argv[cnt++];
  string edge_path = argv[cnt++];
  string feat_path = argv[cnt++];
  string res_path = argv[cnt++];
  string res_log_path = argv[cnt++];
  string query_path = argv[cnt++];
  float r = atof(argv[cnt++]);
  float l = atof(argv[cnt++]);
  int coreness = atoi(argv[cnt++]);
  string cmodel = argv[cnt++];
  string structure_method = argv[cnt++];
  string metric = argv[cnt++];
  int thr_num = atoi(argv[cnt++]);
  int rKACS_out = atoi(argv[cnt++]);

  Cmodel com_base = core;
  if (cmodel == "truss") {
    com_base = truss;
  }
  StructureMethod sMethod = opt2;
  if (structure_method == "baseline") {
    sMethod = baseline;
  }
  __Metric _metric_ = HYB;
  if (metric == "TEXT") {
    _metric_ = TEXT;
  }

  chci::Dataset_Type Dataset_DT = Facebook;
  SWITCH_DSTYPE(dataset_type, DBLP);
  SWITCH_DSTYPE(dataset_type, IMDB_Person);
  SWITCH_DSTYPE(dataset_type, IMDB_Movie);
  SWITCH_DSTYPE(dataset_type, Twitch);
  SWITCH_DSTYPE(dataset_type, LiveJournal);
  SWITCH_DSTYPE(dataset_type, Facebook);
  SWITCH_DSTYPE(dataset_type, FB0);
  // ===================================== Retrieve Stage
  // =======================================
  unique_ptr<CHCIndex> CNNSG;
  switch (Dataset_DT) {
  case chci::DBLP: {
    CNNSG = make_unique<chci_graph<2, 220>>();
  } break;
  case chci::IMDB_Person: {
    CNNSG = make_unique<chci_graph<13, 0>>();
  } break;
  case chci::IMDB_Movie: {
    CNNSG = make_unique<chci_graph<3, 38>>();
  } break;
  case chci::Twitch: {
    CNNSG = make_unique<chci_graph<10, 23>>();
  } break;
  case chci::LiveJournal: {
    CNNSG = make_unique<chci_graph<128, 10>>();
  } break;
  case chci::Facebook: {
    CNNSG = make_unique<chci_graph<0, 1282>>();
    break;
  }
  case chci::FB0: {
    CNNSG = make_unique<chci_graph<0, 1282>>();
    //            CNNSG = make_unique<cnns_graph<0,224>>() ;
    break;
  }
  }
  CNNSG->Load_Graph_bin(Dataset_DT, _metric_, com_base, sMethod, index_p,
                        feat_path, graph_path, vertex_path, edge_path);
  ifstream q_in(query_path);
  if (!q_in) {
    throw(runtime_error("Invalid query path!"));
  }
  vector<CNODEID> querys;
  CNODEID tmp_q;
  while (q_in >> tmp_q) {
    querys.emplace_back(tmp_q);
  }
  q_in.close();
  ofstream f(res_path);
  mutex o_mutex;
  auto retrieve_out = [&f, &o_mutex, rKACS_out,
                       &CNNSG](CNODEID query_id, size_t retrieve_time,
                               vector<CNODEID> &retrieve_res) {
    o_mutex.lock();
    if (rKACS_out == 0) {
      f << query_id << endl;
      for (auto j : retrieve_res) {
        f << j << " ";
      }
      f << endl;
    } else {
      f << CNNSG->ori_2_cnns_map[query_id] << endl;
      for (auto j : retrieve_res) {
        f << CNNSG->ori_2_cnns_map[j] << " ";
      }
      f << endl;
    }
    f << retrieve_time << " mcrs" << endl;
    o_mutex.unlock();
  };

  CNNSG->L_scale = l;
  // #pragma omp parallel for num_threads(thr_num)
  for (auto i = 0; i < querys.size(); ++i) {
    size_t retrieve_time = 0;
    auto query_id = querys[i];
    vector<CNODEID> new_res;
    if (rKACS_out) {
        CNNSG->gamma = 0;
    }
      new_res = CNNSG->Retrieve(query_id, r, coreness, &retrieve_time);
    std::sort(new_res.begin(), new_res.end());
    retrieve_out(query_id, retrieve_time, new_res);
  }
  f.close();
  return 0;
}
