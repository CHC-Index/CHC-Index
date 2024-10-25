
#ifndef CNNS_BASIC_DATA_REBUILD_H
#define CNNS_BASIC_DATA_REBUILD_H

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
#include "basic_define.h"

#define MACHINE_WORDSZ 64
#define PACKSZ 64
namespace chci {
    
    //  General_data is used to store one node's attribute
    template<int numeric_num, int text_num>
    struct General_data {
        std::vector<CCONTINUOUS > numeric_data;
        std::bitset<numeric_num> numeric_bin;
        std::bitset<text_num> text_bin;
        
        General_data() = default;
        template<typename _stl>
        General_data(_stl _numeric_data, std::bitset<text_num> _text_bin)
                : text_bin(std::move(_text_bin)), numeric_data(_numeric_data.begin(), _numeric_data.end()), numeric_bin(0){
            for (auto i = _numeric_data.begin(); i != _numeric_data.end(); ++i){
                if (*i > 0){
                    numeric_bin.set(i-_numeric_data.begin());
                }
            }
        }
    };
    //  64 * 64 PACKED
    // using bitset operation to boost the intersection operation of two nodes' neighbors
    typedef struct two_pressed_pack{
        uint64_t idx2;
        std::vector<uint64_t> val;
        uint32_t idx1;
        two_pressed_pack(uint64_t idx1, uint64_t idx2, std::vector<uint64_t> &val) : idx1(idx1), idx2(idx2), val(val) {}
    } two_pressed_pack;
    // one node's neighbors are packed into packed_neighbor for faster set intersection
    typedef struct packed_neighbor {
//        std::set<CNODEID> neigh_set;
        CNODEID neigh_num = -1;
        CNODEID reordered_id;
        std::vector<two_pressed_pack> neigh_vec;

        packed_neighbor() = default;
        void init(const std::set<CNODEID> &neighbors) {
            neigh_num = neighbors.size();
            if (neigh_num == 0) {
                return;
            }

            uint32_t prv_idx1 = -1;
            uint32_t cur_idx1 = 0;

            std::vector<uint64_t> cur_pack_vec;
            std::bitset<PACKSZ> cur_idx2(0);
            std::bitset<MACHINE_WORDSZ> cur_pack(0);
            auto i = neighbors.begin();
            while (i != neighbors.end()) {
                cur_idx1 = Cal_idx1(*i);
                //  与之前点的idx1相同
                if (cur_idx1 == prv_idx1) {
                    uint32_t cur_idx2_pos = Cal_idx2(*i);
                    //  与之前点的idx2不同：增加一个新word
                    if (!cur_idx2.test(cur_idx2_pos)) {
                        cur_idx2.set(cur_idx2_pos);
                        cur_pack_vec.emplace_back(cur_pack.to_ullong());
                        cur_pack.reset();
                        cur_pack.set(Cal_idx3(*i));
                    } else {
                        //  与之前点的idx2同
                        cur_pack.set(Cal_idx3(*i));
                    }
                } else {
                    //  与之前点的idx1不相同，
                    if (cur_pack.any()) {
                        cur_pack_vec.emplace_back(cur_pack.to_ullong());
                        neigh_vec.emplace_back(prv_idx1, cur_idx2.to_ullong(), cur_pack_vec);
                    }
                    cur_pack_vec.clear();
                    cur_pack.reset();
                    cur_idx2.reset();

                    prv_idx1 = cur_idx1;
                    cur_idx2.set(Cal_idx2(*i));
                    cur_pack.set(Cal_idx3(*i));
                }
                ++i;
            }
            if (cur_pack.any()) {

                cur_pack_vec.emplace_back(cur_pack.to_ullong());
                neigh_vec.emplace_back(prv_idx1, cur_idx2.to_ullong(), cur_pack_vec);
            }
        }
        void init(const std::vector<CNODEID> &neighbors) {
            neigh_num = neighbors.size();
            if (neigh_num == 0) {
                return;
            }

            uint32_t prv_idx1 = -1;
            uint32_t cur_idx1 = 0;

            std::vector<uint64_t> cur_pack_vec;
            std::bitset<PACKSZ> cur_idx2(0);
            std::bitset<MACHINE_WORDSZ> cur_pack(0);
            auto i = neighbors.begin();
            while (i != neighbors.end()) {
                cur_idx1 = Cal_idx1(*i);
                //  与之前点的idx1相同
                if (cur_idx1 == prv_idx1) {
                    uint32_t cur_idx2_pos = Cal_idx2(*i);
                    //  与之前点的idx2不同：增加一个新word
                    if (!cur_idx2.test(cur_idx2_pos)) {
                        cur_idx2.set(cur_idx2_pos);
                        cur_pack_vec.emplace_back(cur_pack.to_ullong());
                        cur_pack.reset();
                        cur_pack.set(Cal_idx3(*i));
                    } else {
                        //  与之前点的idx2同
                        cur_pack.set(Cal_idx3(*i));
                    }
                } else {
                    //  与之前点的idx1不相同，
                    if (cur_pack.any()) {
                        cur_pack_vec.emplace_back(cur_pack.to_ullong());
                        neigh_vec.emplace_back(prv_idx1, cur_idx2.to_ullong(), cur_pack_vec);
                    }
                    cur_pack_vec.clear();
                    cur_pack.reset();
                    cur_idx2.reset();

                    prv_idx1 = cur_idx1;
                    cur_idx2.set(Cal_idx2(*i));
                    cur_pack.set(Cal_idx3(*i));
                }
                ++i;
            }
            if (cur_pack.any()) {

                cur_pack_vec.emplace_back(cur_pack.to_ullong());
                neigh_vec.emplace_back(prv_idx1, cur_idx2.to_ullong(), cur_pack_vec);
            }
        }

        inline uint32_t Cal_idx1(uint32_t pos) {
            return pos >> 12;
        }

        inline uint32_t Cal_idx2(uint32_t pos) {
            return (pos & 0xfff) >> 6;
        }

        inline uint32_t Cal_idx3(uint32_t pos) {
            return pos & 0x3f;
        }

        bool id_in(const CNODEID node_reordered_id){
            if (neigh_num + 1 == 0) {
                return false;
            }
            uint32_t idx1 = Cal_idx1(node_reordered_id);
            int position = pos(idx1);
            if (position == -1) {
                return false;
            } else {
                uint32_t idx2_pos = Cal_idx2(node_reordered_id);
                uint32_t idx3_pos = Cal_idx3(node_reordered_id);
                uint64_t cur_idx2_pos = (neigh_vec[position].idx2 & (uint64_t(1) << idx2_pos));
                if (cur_idx2_pos){
                    int pos2 = __builtin_popcountll((~((~cur_idx2_pos)+1)) & neigh_vec[position].idx2) ;
                    if ((neigh_vec[position].val[pos2] >> idx3_pos) & 1)
                        return true;
                    else
                        return false;
                }
                return false;
            }
        }

        inline int pos(uint32_t idx1){
            int less = 0;
            int greater = neigh_vec.size() - 1;
            int mid = 0;
            while (true) {
                mid = (less + greater) / 2;
                if (idx1 == neigh_vec[mid].idx1) {
                    return mid;
                } else if (idx1 < neigh_vec[mid].idx1) {
                    greater = mid - 1;
                } else {
                    less = mid + 1;
                }
                if (less > greater) {
                    return -1;
                }
            }
        }
    } packed_neighbor;
    
    typedef struct Structure_data : packed_neighbor {
    public:
        CNODEID cnns_id = -1;

        Structure_data() = default;
        Structure_data(CNODEID cnns_id_in, std::vector<CNODEID> &neighbor_in, std::vector<CNODEID> &cnns_2_reord) : cnns_id(cnns_id_in) {
            reordered_id = cnns_2_reord[cnns_id_in];
            std::set<CNODEID> reordered_neighbor;
            for (auto i: neighbor_in) {
                reordered_neighbor.emplace(cnns_2_reord[i]);
            }
            init(reordered_neighbor);
        }
        
    } Structure_data;
    
    
    
}
#endif //CNNS_BASIC_DATA_H
