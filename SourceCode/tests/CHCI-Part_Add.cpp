#include "chci.h"
#define SWITCH_DSTYPE(in, str) if (in == TOSTRING(str)) Dataset_DT =str

using namespace chci;
using namespace std;
int main(int argc, char** argv){
    std::cout << "Begin Formal Test, Stage Build Index!\n";

    int cnt = 1;
    std::string dataset_type = argv[cnt++];
    string index_p = argv[cnt++];
    string graph_path = argv[cnt++];
    string vertex_path = argv[cnt++];
    string edge_path = argv[cnt++];
    string feat_path = argv[cnt++];
    string ori_valid_node_path = argv[cnt++];
    string add_node_path = argv[cnt++];
    string new_index_path = argv[cnt++];
    int max_deg = atoi(argv[cnt++]);
    int build_L = atoi(argv[cnt++]);
    float alpha = atof(argv[cnt++]);
    float B = atof(argv[cnt++]);
    bool activate_rppp = atoi(argv[cnt++]) > 0;
    int num_threads = atoi(argv[cnt++]);
    string cmodel = argv[cnt++];
    string structure_method = argv[cnt++];
    string metric = argv[cnt++];


    Cmodel com_base = core;
    if (cmodel == "truss"){
        com_base = truss;
    }
    StructureMethod sMethod = opt2;
    if (structure_method == "baseline"){
        sMethod = baseline;
    }
    __Metric _metric_ = HYB;
    if (metric == "TEXT"){
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
// ===================================== Build Stage =======================================
    unique_ptr<CHCIndex> CNNSG;
    switch (Dataset_DT) {
        case chci::DBLP: {
            CNNSG = make_unique<chci_graph<2,220>>() ;
        }
            break;
        case chci::IMDB_Person: {
            CNNSG = make_unique<chci_graph<13,0>>() ;
        }
            break;
        case chci::IMDB_Movie: {
            CNNSG = make_unique<chci_graph<3,38>>() ;
        }
            break;
        case chci::Twitch: {
            CNNSG = make_unique<chci_graph<10,23>>() ;
        }
            break;
        case chci::LiveJournal: {
            CNNSG = make_unique<chci_graph<4,4>>() ;
        }
            break;
        case chci::Facebook: {
                    CNNSG = make_unique<chci_graph<0,1282>>() ;
                    break;
            }
        case chci::FB0: {
            CNNSG = make_unique<chci_graph<0,1282>>() ;
//            CNNSG = make_unique<cnns_graph<0,224>>() ;
            break;
        }
    }
    CNNSG->Load_Graph_bin_part(Dataset_DT, _metric_, com_base, sMethod,  index_p, feat_path, graph_path, vertex_path, edge_path, ori_valid_node_path);
    std::cout << "Index would be saved into dir: " << new_index_path << std::endl;
    auto s = CLOCK;
    CNNSG->Batch_Add(add_node_path);
    CNNSG->Save_Graph_bin(new_index_path);
    auto e = CLOCK;
    std::cout << "Add Nodes Time: " << std::chrono::duration_cast<std::chrono::seconds>(e - s).count() << std::endl;
    std::cout << "Index would be saved into dir: " << new_index_path << std::endl;
    return 0;
}