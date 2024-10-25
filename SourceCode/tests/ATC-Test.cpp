#include "ATC.h"
using namespace chci;
using namespace std;

#define SWITCH_DSTYPE(in, str)                                                 \
  if (in == TOSTRING(str))                                                     \
  Dataset_DT = chci::str

int main(int argc, char **argv) {
  int cnt = 1;
  std::string dataset_type = argv[cnt++];
  string index_path = argv[cnt++];
  string graph_path = argv[cnt++];
  string vertex_path = argv[cnt++];
  string edge_path = argv[cnt++];
  string feat_path = argv[cnt++];
  string queryRes = argv[cnt++];
  string ATCqueryRes = argv[cnt++];
  string ATCqueryRes_WO = argv[cnt++];
  int coreness = atoi(argv[cnt++]);
  int d = atoi(argv[cnt++]);
  float gamma = atof(argv[cnt++]);
  int run_with = atoi(argv[cnt++]);
  int run_wo = atoi(argv[cnt++]);
  int search_threads = atoi(argv[cnt++]);

  std::cout << dataset_type;

  chci::Dataset_Type Dataset_DT = Facebook;
  SWITCH_DSTYPE(dataset_type, DBLP);
  SWITCH_DSTYPE(dataset_type, IMDB_Person);
  SWITCH_DSTYPE(dataset_type, IMDB_Movie);
  SWITCH_DSTYPE(dataset_type, Twitch);
  SWITCH_DSTYPE(dataset_type, LiveJournal);
  SWITCH_DSTYPE(dataset_type, Facebook);
  SWITCH_DSTYPE(dataset_type, FB0);

  std::cout << "Begin ATC Algorithm!\n";
  switch (Dataset_DT) {
  case chci::DBLP: {
    chci::atc<2, 220> ATC;
    ATC.Load_Index(Dataset_DT, index_path, feat_path, graph_path, vertex_path,
                   edge_path);

    ATC.Query_More_Nodes_withIndex_Formal_2cho(
        queryRes, ATCqueryRes, ATCqueryRes_WO, coreness, d, search_threads,
        run_with, run_wo, gamma);
  } break;
  case chci::IMDB_Person: {
    chci::atc<13, 0> ATC;
    ATC.Load_Index(Dataset_DT, index_path, feat_path, graph_path, vertex_path,
                   edge_path);

    ATC.Query_More_Nodes_withIndex_Formal_2cho(
        queryRes, ATCqueryRes, ATCqueryRes_WO, coreness, d, search_threads,
        run_with, run_wo, gamma);
  } break;
  case chci::IMDB_Movie: {
    chci::atc<3, 38> ATC;
    ATC.Load_Index(Dataset_DT, index_path, feat_path, graph_path, vertex_path,
                   edge_path);

    ATC.Query_More_Nodes_withIndex_Formal_2cho(
        queryRes, ATCqueryRes, ATCqueryRes_WO, coreness, d, search_threads,
        run_with, run_wo, gamma);
  } break;
  case chci::Twitch: {
    chci::atc<10, 23> ATC;
    ATC.Load_Index(Dataset_DT, index_path, feat_path, graph_path, vertex_path,
                   edge_path);
    ATC.Query_More_Nodes_withIndex_Formal_2cho(
        queryRes, ATCqueryRes, ATCqueryRes_WO, coreness, d, search_threads,
        run_with, run_wo, gamma);
  } break;
  case chci::LiveJournal: {
    chci::atc<128, 12> ATC;
    ATC.Load_Index(Dataset_DT, index_path, feat_path, graph_path, vertex_path,
                   edge_path);
    ATC.Query_More_Nodes_withIndex_Formal_2cho(
        queryRes, ATCqueryRes, ATCqueryRes_WO, coreness, d, search_threads,
        run_with, run_wo, gamma);
  } break;
  case chci::Facebook: {
    chci::atc<0, 1282> ATC;
    ATC.Load_Index(Dataset_DT, index_path, feat_path, graph_path, vertex_path,
                   edge_path);
    ATC.Query_More_Nodes_withIndex_Formal_2cho(
        queryRes, ATCqueryRes, ATCqueryRes_WO, coreness, d, search_threads,
        run_with, run_wo, gamma);
    break;
  }
  case chci::FB0: {
    chci::atc<0, 224> ATC;
    ATC.Load_Index(Dataset_DT, index_path, feat_path, graph_path, vertex_path,
                   edge_path);
    ATC.Query_More_Nodes_withIndex_Formal_2cho(
        queryRes, ATCqueryRes, ATCqueryRes_WO, coreness, d, search_threads,
        run_with, run_wo, gamma);
    break;
  }
  }
    
    std::cout << "Result would be stored into: " << ATCqueryRes <<std::endl;
  return 0;
}
