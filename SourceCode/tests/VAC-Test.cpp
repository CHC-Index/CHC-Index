#include "VAC.h"

#define SWITCH_DSTYPE(in, str)                                                 \
  if (in == TOSTRING(str))                                                     \
  Dataset_DT = chci::str
using namespace std;
using namespace chci;
int main(int argc, char **argv) {
  std::cout << "Begin VAC... \n";
  int cnt = 1;
  std::string dataset_type = argv[cnt++];
  string index_path = argv[cnt++];
  string graph_path = argv[cnt++];
  string vertex_path = argv[cnt++];
  string edge_path = argv[cnt++];
  string feat_path = argv[cnt++];
  string queryRes = argv[cnt++];
  string VACqueryRes = argv[cnt++];
  string VACqueryRes_wo = argv[cnt++];
  int coreness = atoi(argv[cnt++]);
  int run_with = atoi(argv[cnt++]);
  int run_wo = atoi(argv[cnt++]);
  int search_threads = atoi(argv[cnt++]);

  chci::Dataset_Type Dataset_DT = Facebook;
  SWITCH_DSTYPE(dataset_type, DBLP);
  SWITCH_DSTYPE(dataset_type, IMDB_Person);
  SWITCH_DSTYPE(dataset_type, IMDB_Movie);
  SWITCH_DSTYPE(dataset_type, Twitch);
  SWITCH_DSTYPE(dataset_type, LiveJournal);
  SWITCH_DSTYPE(dataset_type, Facebook);
  SWITCH_DSTYPE(dataset_type, FB0);

  std::cout << "Begin Formal Test, Stage Search ATC!\n";
  std::cout << "Query Result : " << queryRes << std::endl;
  switch (Dataset_DT) {
  case chci::DBLP: {
    chci::vac<2, 220> VAC;
    VAC.Load_Index(Dataset_DT, index_path, feat_path, graph_path, vertex_path,
                   edge_path);
    VAC.Query_More_Nodes_withIndex_Formal(queryRes, VACqueryRes, VACqueryRes_wo,
                                          coreness, search_threads, run_with,
                                          run_wo);
  } break;
  case chci::IMDB_Person: {
    chci::vac<13, 0> VAC;
    VAC.Load_Index(Dataset_DT, index_path, feat_path, graph_path, vertex_path,
                   edge_path);
    VAC.Query_More_Nodes_withIndex_Formal(queryRes, VACqueryRes, VACqueryRes_wo,
                                          coreness, search_threads, run_with,
                                          run_wo);
  } break;
  case chci::IMDB_Movie: {

    chci::vac<3, 38> VAC;
    VAC.Load_Index(Dataset_DT, index_path, feat_path, graph_path, vertex_path,
                   edge_path);
    VAC.Query_More_Nodes_withIndex_Formal(queryRes, VACqueryRes, VACqueryRes_wo,
                                          coreness, search_threads, run_with,
                                          run_wo);
  } break;
  case chci::Twitch: {
    chci::vac<10, 23> VAC;
    VAC.Load_Index(Dataset_DT, index_path, feat_path, graph_path, vertex_path,
                   edge_path);
    VAC.Query_More_Nodes_withIndex_Formal(queryRes, VACqueryRes, VACqueryRes_wo,
                                          coreness, search_threads, run_with,
                                          run_wo);
  } break;
  case chci::LiveJournal: {
    chci::vac<128, 12> VAC;
    VAC.Load_Index(Dataset_DT, index_path, feat_path, graph_path, vertex_path,
                   edge_path);
    VAC.Query_More_Nodes_withIndex_Formal(queryRes, VACqueryRes, VACqueryRes_wo,
                                          coreness, search_threads, run_with,
                                          run_wo);
  } break;
  case chci::Facebook: {
    chci::vac<0, 1282> VAC;
    VAC.Load_Index(Dataset_DT, index_path, feat_path, graph_path, vertex_path,
                   edge_path);
    VAC.Query_More_Nodes_withIndex_Formal(queryRes, VACqueryRes, VACqueryRes_wo,
                                          coreness, search_threads, run_with,
                                          run_wo);
    break;
  }
  case chci::FB0: {
    chci::vac<0, 1282> VAC;
    //            cnns::vac<0,224> VAC;
    VAC.Load_Index(Dataset_DT, index_path, feat_path, graph_path, vertex_path,
                   edge_path);
    VAC.Query_More_Nodes_withIndex_Formal(queryRes, VACqueryRes, VACqueryRes_wo,
                                          coreness, search_threads, run_with,
                                          run_wo);
    break;
  }
  }

  return 0;
}
