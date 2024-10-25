// Some useful functions
#ifndef UTILS_H
#define UTILS_H

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
#include "basic_data.h"

namespace chci {

    struct cmp {
        constexpr bool
        operator()(std::pair<CDEGREE, CNODEID> const &a,
                   std::pair<CDEGREE, CNODEID> const &b) const noexcept {
            return a.first < b.first;
        }

        constexpr bool
        operator()(std::pair<CDIST, CNODEID> const &a,
                   std::pair<CDIST, CNODEID> const &b) const noexcept {
            return a.first < b.first;
        }
        
        constexpr bool
        operator()(std::pair<CNODEID , CNODEID> const &a,
                   std::pair<CNODEID , CNODEID> const &b) const noexcept {
            return a.first < b.first;
        }
    };


    template<typename T>
    using maxHeap = std::priority_queue<std::pair<T, CNODEID>, std::vector<std::pair<T, CNODEID>>, cmp>;

    template<typename T>
    std::ofstream &operator<<(std::ofstream &out, const std::vector<T> &A) {
        for (auto i = 0; i < A.size(); i++) {
            out << A[i] << " ";
        }
        return out;
    }

    template<typename T1, typename T2>
    std::ofstream &operator<<(std::ofstream &out, const std::vector<std::pair<T1, T2>> &A) {
        for (auto i = 0; i < A.size(); i++) {
            out << A[i].first << " ";
            out << A[i].second << " ";
        }
        return out;
    }

    inline std::ostream &operator<<(std::ostream &out, const Dataset_Type value) {
        static std::map<Dataset_Type, std::string> strs;
        if (strs.size() == 0) {
#define INSERT_ELEM(p) strs[p] = #p
            INSERT_ELEM(small_dblp);
            INSERT_ELEM(DBLP);
#undef INSERT_ELEM
        }
        out << strs[value];
        return out;
    }


#ifndef CASE_DATASET
#define CASE_DATASET(str) case str : return #str; break;
    std::string Print_Dataset_Type(Dataset_Type dt){
        switch (dt) {
            CASE_DATASET(small_dblp)
            CASE_DATASET(DBLP)
            CASE_DATASET(IMDB_Person)
            CASE_DATASET(IMDB_Movie)
            CASE_DATASET(FourSquare)
            CASE_DATASET(CORA)
            CASE_DATASET(LiveJournal)
            CASE_DATASET(Twitch)
            CASE_DATASET(Facebook)
            CASE_DATASET(FB0)
            CASE_DATASET(DBpedia_Automobile)
        }
        return "UNKNOWN DATASET!!";
    }

#endif
};
#endif
