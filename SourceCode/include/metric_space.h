// The code in this file is used for distance calculation during the process of constructing the proximity graph.
#ifndef CS_ON_ANNS_METRIC_SPACE_H
#define CS_ON_ANNS_METRIC_SPACE_H

#include <cmath>
#include <unordered_set>
#include <algorithm>
#include <bitset>
#include "basic_define.h"
#include "basic_data.h"


//#pragma once
namespace chci {
    template <int numeric_num, int text_num>
    class HYBSpace {
    public:
        chci::Dataset_Type datasetType;
        chci::__Metric __metric;
        int rs_scale_val = 0;
        
        HYBSpace() = default;
        HYBSpace(Dataset_Type datasetType_in, __Metric _metric_) : datasetType(datasetType_in), __metric(_metric_) {
            rs_scale_val = numeric_num;
        }
        
        template<typename T>
        float JaccardDist(const std::vector<T> &a, const std::vector<T> &b) {
            float dis = 0;
            int cnt = 0;
            assert(a.size() == b.size());
            int total = a.size();
            for (int i = 0; i < total; i++) {
                if (a[i] == b[i])
                    cnt++;
            }
            dis = static_cast<float>(cnt) / static_cast<float>(total);
            return dis;
        }
        
        template<int num>
        inline CDIST JaccardDist1(const std::bitset<num> &binA,const  std::bitset<num> &binB) const {
            if (binA.count() == 0 || binB.count() == 0) {
                return 1.0;
            }
            return (1 - static_cast<float >((binA & binB).count()) / static_cast<float >((binA | binB).count()));
        }
        
        template<int num1, int num2>
        inline CDIST
        JaccardDist(const std::bitset<num1>& A1, const std::bitset<num1>& B1, const std::bitset<num2>& A2, const std::bitset<num2>& B2) const{
            CUINT upper = 0;
            CUINT lower = 0;
            upper = (A1 & B1).count() + (A2 & B2).count();
            lower = (A1 | B1).count() + (A2 | B2).count();
            float res = 1;
            if (lower != 0) {
                res = 1 - static_cast<float>(upper) / static_cast<float>(lower);
            }
            return res;
        }
        
        inline float ManhatDist(float a, float b) {
            
            float res = a - b;
            if (res < 0)
                res = -res;
            return res;
            
        }
        
        template<typename _stl>
        inline CDIST ManhatDist(const _stl a,const  _stl b) const{
            CDIST res = 0;
            for (auto i = a.begin(), j = b.begin(); i != a.end(); ++i , ++j){
                res += std::abs(*i-*j);
            }
            return res;
        }
        
        inline CDIST neighDist(const CNODEID a, const Structure_data *pn1,const CNODEID b, const Structure_data *pn2) const {
            if (!(pn1->neigh_num && pn2->neigh_num)) {
                return 6.0;
            }

//            if (pn1->neighbor.count(b) || a == b){
//                return 0;
//            }
//            if (const_cast<Structure_data *>(pn1)->id_in(b) || a == b) {
//                return 0;
//            }

            CNODEID cnt = 0;
            auto first1 = pn1->neigh_vec.begin();
            auto last1 = pn1->neigh_vec.end();
            auto first2 = pn2->neigh_vec.begin();
            auto last2 = pn2->neigh_vec.end();

            while (first1 != last1 && first2 != last2) {
                if (first1->idx1 < first2->idx1) ++first1;
                else if (first2->idx1 < first1->idx1) ++first2;
                else {
                    cnt += Deal_2_pressed_o_64(*first1, *first2);
                    ++first1;
                    ++first2;
                }
            }
            float res = 0;
            if (cnt == 0){
                res += 4;
            } else if (cnt > 0 && !const_cast<Structure_data *>(pn1)->id_in(b) && a != b){
                res += 2;
            }
            res += 1.0 - static_cast<CCONTINUOUS >(cnt) / static_cast<CCONTINUOUS >(pn1->neigh_num);
//            res += 1.0 - static_cast<CCONTINUOUS >(cnt) / static_cast<CCONTINUOUS >(pn1->neigh_num+pn2->neigh_num-cnt) ;
//            float res = 1.0 - static_cast<CCONTINUOUS >(cnt) / static_cast<CCONTINUOUS >(pn2->neigh_num) ;
            return res;
        }

        inline CDIST neighDist_search(const CNODEID a, const Structure_data *pn1,const CNODEID b, const Structure_data *pn2) const {
            CNODEID cnt = 0;
            auto first1 = pn1->neigh_vec.begin();
            auto last1 = pn1->neigh_vec.end();
            auto first2 = pn2->neigh_vec.begin();
            auto last2 = pn2->neigh_vec.end();

            while (first1 != last1 && first2 != last2) {
                if (first1->idx1 < first2->idx1) ++first1;
                else if (first2->idx1 < first1->idx1) ++first2;
                else {
                    cnt += Deal_2_pressed_o_64(*first1, *first2);
                    ++first1;
                    ++first2;
                }
            }
            float res = 0;

            res = 1.0 - static_cast<CCONTINUOUS >(cnt) / static_cast<CCONTINUOUS >(pn1->neigh_num);
//            res += 1.0 - static_cast<CCONTINUOUS >(cnt) / static_cast<CCONTINUOUS >(pn1->neigh_num+pn2->neigh_num-cnt) ;
//            float res = 1.0 - static_cast<CCONTINUOUS >(cnt) / static_cast<CCONTINUOUS >(pn2->neigh_num) ;
            return res;
        }


        inline int Deal_2_pressed_o_64(const two_pressed_pack &a, const two_pressed_pack &b) const {
            int res = 0;
            std::bitset<64> intersection = a.idx2 & b.idx2;
            if (!intersection.any()) {
                return 0;
            }

            auto a_vec_itr = (const_cast<two_pressed_pack &>(a).val.begin());
            auto b_vec_itr = const_cast<two_pressed_pack &>(b).val.begin();

            uint32_t inter_itr_b = 0;
            uint32_t inter_itr_f = intersection._Find_first();
            uint64_t mask_bit;
//            uint32_t high_bits_mask = (0xffffffff << (inter_itr_f));
//            uint32_t low_bits_mask = 0;
//            mask_bit = ~(high_bits_mask | low_bits_mask);
            mask_bit = ~((0xffffffffffffffff << (inter_itr_f)));
            while (inter_itr_f != 64) {
                int a_plus = __builtin_popcountll(a.idx2 & mask_bit);
                int b_plus = __builtin_popcountll(b.idx2 & mask_bit);

                a_vec_itr += a_plus;
                b_vec_itr += b_plus;
                res += __builtin_popcountll(*a_vec_itr & *b_vec_itr);

                inter_itr_b = inter_itr_f;
                inter_itr_f = intersection._Find_next(inter_itr_b);
                mask_bit = ~((0xffffffffffffffff << (inter_itr_f)) | ((0xffffffffffffffff >> (63 - inter_itr_b)) >> 1));
//                high_bits_mask = (0xffffffff << (inter_itr_f));
//                low_bits_mask = (0xffffffff >> (32 - inter_itr_b));
//                mask_bit ^= (high_bits_mask | low_bits_mask);
//                mask_bit = ~(high_bits_mask | low_bits_mask);
            }
            return res;
        }

        inline int Deal_2_pressed_o_32(const two_pressed_pack &a, const two_pressed_pack &b) {
            int res = 0;
            std::bitset<32> intersection = a.idx2 & b.idx2;
            if (!intersection.any()) {
                return 0;
            }

            auto a_vec_itr = (const_cast<two_pressed_pack &>(a).val.begin());
            auto b_vec_itr = const_cast<two_pressed_pack &>(b).val.begin();

            uint32_t inter_itr_b = 0;
            uint32_t inter_itr_f = intersection._Find_first();
            uint32_t mask_bit;
//            uint32_t high_bits_mask = (0xffffffff << (inter_itr_f));
//            uint32_t low_bits_mask = 0;
//            mask_bit = ~(high_bits_mask | low_bits_mask);
            mask_bit = ~((0xffffffff << (inter_itr_f)));
            while (inter_itr_f != 32) {
                int a_plus = __builtin_popcountll(a.idx2 & mask_bit);
                int b_plus = __builtin_popcountll(b.idx2 & mask_bit);

                a_vec_itr += a_plus;
                b_vec_itr += b_plus;
                res += __builtin_popcountll(*a_vec_itr & *b_vec_itr);

                inter_itr_b = inter_itr_f;
                inter_itr_f = intersection._Find_next(inter_itr_b);
                mask_bit = ~((0xffffffff << (inter_itr_f)) | ((0xffffffff >> (31 - inter_itr_b)) >> 1));
//                high_bits_mask = (0xffffffff << (inter_itr_f));
//                low_bits_mask = (0xffffffff >> (32 - inter_itr_b));
//                mask_bit ^= (high_bits_mask | low_bits_mask);
//                mask_bit = ~(high_bits_mask | low_bits_mask);
            }
            return res;
        }

//   ***  Origin 64-pack metric *** //
//    float neighDist(CNODEID a, const Structure_data *pn1, CNODEID b, const Structure_data *pn2) {
//        if (!(pn1->neigh_num && pn2->neigh_num)){
//            return 1.0;
//        }
//        if (pn1->neighbor.count(b)){
//            return 0;
//        }
//
//        CNODEID cnt = 0;
//        auto first1 = pn1->wordPack.begin();
//        auto last1 = pn1->wordPack.end();
//        auto first2 = pn2->wordPack.begin();
//        auto last2 = pn2->wordPack.end();
//
//            while (first1!=last1 && first2!=last2)
//            {
//                if (first1->first<first2->first) ++first1;
//                else if (first2->first<first1->first) ++first2;
//                else {
//                    cnt += __builtin_popcountll(first1->second & first2->second);
//                    ++first1; ++first2;
//                }
//            }
//        float res = 1.0 - static_cast<CCONTINUOUS >(cnt)/static_cast<CCONTINUOUS >(pn1->neigh_num);
//        return res;
//    }
        
        inline CDIST full_dist(const General_data<numeric_num, text_num> &a_d,const  General_data<numeric_num, text_num> &b_d
                                             ,const   Structure_data *a_s,const   Structure_data *b_s) const {
            CDIST res = -99999;
            switch (__metric) {
                case HYB:
                    res = (ManhatDist(a_d.numeric_data, b_d.numeric_data) + rs_scale_val* JaccardDist1<text_num>(a_d.text_bin, b_d.text_bin))/(2*rs_scale_val) +
                            neighDist(a_s->reordered_id, a_s, b_s->reordered_id, b_s);
                    break;
                case TEXT:
                    res = JaccardDist<numeric_num, text_num>(a_d.numeric_bin, b_d.numeric_bin, a_d.text_bin, b_d.text_bin) + neighDist(a_s->reordered_id, a_s, b_s->reordered_id, b_s);
                    break;
            }
            if (res < 0){
                throw std::runtime_error("Distance is not valid!!!!!");
            }
            return res;
        }

        inline CDIST full_dist_retrieve(const General_data<numeric_num, text_num> &a_d,const  General_data<numeric_num, text_num> &b_d
                ,const   Structure_data *a_s,const   Structure_data *b_s) const {
            CDIST res = -99999;
            switch (__metric) {
                case HYB:
                    res = (ManhatDist(a_d.numeric_data, b_d.numeric_data) + rs_scale_val* JaccardDist1<text_num>(a_d.text_bin, b_d.text_bin))/(2*rs_scale_val) +
                          neighDist_search(a_s->reordered_id, a_s, b_s->reordered_id, b_s);
                    break;
                case TEXT:
                    res = JaccardDist<numeric_num, text_num>(a_d.numeric_bin, b_d.numeric_bin, a_d.text_bin, b_d.text_bin) + neighDist_search(a_s->reordered_id, a_s, b_s->reordered_id, b_s);
                    break;
            }
            if (res < 0){
                throw std::runtime_error("Distance is not valid!!!!!");
            }
            return res;
        }

        inline CDIST full_dist(const General_data<0, text_num> &a_d,const  General_data<0, text_num> &b_d
                ,const   Structure_data *a_s,const   Structure_data *b_s) const {
            CDIST res = -99999;
            res = JaccardDist1<text_num>(a_d.text_bin, b_d.text_bin) + neighDist(a_s->reordered_id, a_s, b_s->reordered_id, b_s);
            if (res < 0){
                throw std::runtime_error("Distance is not valid!!!!!");
            }
            return res;
        }
        
        inline CCONTINUOUS dist_Find_Enter_pt(const std::vector<CCONTINUOUS>& vec1,const  General_data<numeric_num, text_num> data1) const{
            CCONTINUOUS res =0;
            for (auto i = 0; i < numeric_num; ++i){
                res += std::abs(vec1[i]-data1.numeric_data[i]);
            }
            for (auto i = 0; i < text_num; ++i){
                res += std::abs(vec1[i+numeric_num]-data1.text_bin[i]);
            }
            return res;
        }
        
        inline CCONTINUOUS full_dist_noNeigh(const General_data<numeric_num, text_num> &a_d, const General_data<numeric_num, text_num> &b_d){
            CCONTINUOUS res = -99999;
            switch (__metric) {
                case HYB:
                    res = (ManhatDist(a_d.numeric_data, b_d.numeric_data) + rs_scale_val* JaccardDist1<text_num>(a_d.text_bin, b_d.text_bin))/(2*rs_scale_val);
                    break;
                case TEXT:
                    res = JaccardDist<numeric_num, text_num>(a_d.numeric_bin, b_d.numeric_bin, a_d.text_bin, b_d.text_bin) ;
                    break;
            }
            if (res < 0){
                throw std::runtime_error("Distance is not valid!!!!!");
            }
            return res;
        }
        
        
        int rs_scale() {
            int res = -9999;
            switch (datasetType) {
                case IMDB_Person:
                    res = 13;
                    break;
                case IMDB_Movie:
                    res = 6;
                    break;
                case DBLP:
//                res =  full_dist(reinterpret_cast<const basic_node_data*>(A), reinterpret_cast<const basic_node_data*>(B));
                    res = 4;
                    break;
                case small_dblp:
                    res = 4;
                    break;
                case FourSquare:
                    res = 4;
                    break;
                case CORA:
                    res = 1;
                    break;
                case Twitch:
                    res = 20;
                    break;
                case LiveJournal:
                    res = 16;
                    break;
                case DBpedia_Soccerplayer:
                    res = 7;
                    break;
                case Github:
                    res = 1;
                    break;
            }
            return res;
        }
        
    };
    
    template <int text_num>
    class HYBSpace<0, text_num> {
    public:
        chci::Dataset_Type datasetType;
        chci::__Metric __metric;
        int rs_scale_val = 0;
        
        HYBSpace() = default;
        HYBSpace(Dataset_Type datasetType_in, __Metric _metric_) : datasetType(datasetType_in), __metric(_metric_) {
            rs_scale_val = 0;
        }
        
        template<typename T>
        float JaccardDist(const std::vector<T> &a, const std::vector<T> &b) {
            float dis = 0;
            int cnt = 0;
            assert(a.size() == b.size());
            int total = a.size();
            for (int i = 0; i < total; i++) {
                if (a[i] == b[i])
                    cnt++;
            }
            dis = static_cast<float>(cnt) / static_cast<float>(total);
            return dis;
        }
        
        template<int num>
        inline CDIST JaccardDist(const std::bitset<num> &binA,const  std::bitset<num> &binB) const {
            if (binA.count() == 0 || binB.count() == 0) {
                return 1.0;
            }
            return (1 - static_cast<float >((binA & binB).count()) / static_cast<float >((binA | binB).count()));
        }
        
        template<int num1, int num2>
        inline CDIST
        JaccardDist(const std::bitset<num1>& A1, const std::bitset<num1>& B1, const std::bitset<num2>& A2, const std::bitset<num2>& B2) const{
            CUINT upper = 0;
            CUINT lower = 0;
            upper = (A1 & B1).count() + (A2 & B2).count();
            lower = (A1 | B1).count() + (A2 | B2).count();
            float res = 1;
            if (lower != 0) {
                res = 1 - static_cast<float>(upper) / static_cast<float>(lower);
            }
            return res;
        }
        
        inline float ManhatDist(float a, float b) {
            
            float res = a - b;
            if (res < 0)
                res = -res;
            return res;
            
        }
        
        template<typename _stl>
        inline CDIST ManhatDist(const _stl a,const  _stl b) const{
            CDIST res = 0;
            for (auto i = a.begin(), j = b.begin(); i != a.end(); ++i , ++j){
                res += std::abs(*i-*j);
            }
            return res;
        }

//  ****************  2-packed metric ******************  //

        inline CDIST neighDist(const CNODEID a, const Structure_data *pn1,const CNODEID b, const Structure_data *pn2) const {
            if (!(pn1->neigh_num && pn2->neigh_num)) {
                return 6.0;
            }

//            if (pn1->neighbor.count(b) || a == b){
//                return 0;
//            }
//            if (const_cast<Structure_data *>(pn1)->id_in(b) || a == b) {
//                return 0;
//            }

            CNODEID cnt = 0;
            auto first1 = pn1->neigh_vec.begin();
            auto last1 = pn1->neigh_vec.end();
            auto first2 = pn2->neigh_vec.begin();
            auto last2 = pn2->neigh_vec.end();

            while (first1 != last1 && first2 != last2) {
                if (first1->idx1 < first2->idx1) ++first1;
                else if (first2->idx1 < first1->idx1) ++first2;
                else {
                    cnt += Deal_2_pressed_o_64(*first1, *first2);
                    ++first1;
                    ++first2;
                }
            }
            float res = 0;
            if (cnt == 0){
                res += 4;
            } else if (cnt > 0 && !const_cast<Structure_data *>(pn1)->id_in(b) && a != b){
                res += 2;
            }
            res += 1.0 - static_cast<CCONTINUOUS >(cnt) / static_cast<CCONTINUOUS >(pn1->neigh_num);
//            res += 1.0 - static_cast<CCONTINUOUS >(cnt) / static_cast<CCONTINUOUS >(pn1->neigh_num+pn2->neigh_num-cnt) ;
//            float res = 1.0 - static_cast<CCONTINUOUS >(cnt) / static_cast<CCONTINUOUS >(pn2->neigh_num) ;
            return res;
        }
//
//        inline CDIST neighDist(const CNODEID a, const Structure_data *pn1,const CNODEID b, const Structure_data *pn2) const {
//            if (!(pn1->neigh_num && pn2->neigh_num)) {
//                return 1.0;
//            }
//
//            if (const_cast<Structure_data *>(pn1)->id_in(b) || a == b) {
//                return 0;
//            }
//
//            CNODEID cnt = 0;
//            auto first1 = pn1->neigh_vec.begin();
//            auto last1 = pn1->neigh_vec.end();
//            auto first2 = pn2->neigh_vec.begin();
//            auto last2 = pn2->neigh_vec.end();
//
//            while (first1 != last1 && first2 != last2) {
//                if (first1->idx1 < first2->idx1) ++first1;
//                else if (first2->idx1 < first1->idx1) ++first2;
//                else {
//                    cnt += Deal_2_pressed_o_64(*first1, *first2);
//                    ++first1;
//                    ++first2;
//                }
//            }
//            float res = 1.0 - static_cast<CCONTINUOUS >(cnt) / static_cast<CCONTINUOUS >(pn1->neigh_num);
//            return res;
//        }

        inline CDIST neighDist_search(const CNODEID a, const Structure_data *pn1,const CNODEID b, const Structure_data *pn2) const {
            CNODEID cnt = 0;
            auto first1 = pn1->neigh_vec.begin();
            auto last1 = pn1->neigh_vec.end();
            auto first2 = pn2->neigh_vec.begin();
            auto last2 = pn2->neigh_vec.end();

            while (first1 != last1 && first2 != last2) {
                if (first1->idx1 < first2->idx1) ++first1;
                else if (first2->idx1 < first1->idx1) ++first2;
                else {
                    cnt += Deal_2_pressed_o_64(*first1, *first2);
                    ++first1;
                    ++first2;
                }
            }
            float res = 0;

            res = 1.0 - static_cast<CCONTINUOUS >(cnt) / static_cast<CCONTINUOUS >(pn1->neigh_num);
//            res += 1.0 - static_cast<CCONTINUOUS >(cnt) / static_cast<CCONTINUOUS >(pn1->neigh_num+pn2->neigh_num-cnt) ;
//            float res = 1.0 - static_cast<CCONTINUOUS >(cnt) / static_cast<CCONTINUOUS >(pn2->neigh_num) ;
            return res;
        }

        inline int Deal_2_pressed_o_64(const two_pressed_pack &a, const two_pressed_pack &b) const {
            int res = 0;
            std::bitset<64> intersection = a.idx2 & b.idx2;
            if (!intersection.any()) {
                return 0;
            }
            
            auto a_vec_itr = (const_cast<two_pressed_pack &>(a).val.begin());
            auto b_vec_itr = const_cast<two_pressed_pack &>(b).val.begin();
            
            uint32_t inter_itr_b = 0;
            uint32_t inter_itr_f = intersection._Find_first();
            uint64_t mask_bit;
//            uint32_t high_bits_mask = (0xffffffff << (inter_itr_f));
//            uint32_t low_bits_mask = 0;
//            mask_bit = ~(high_bits_mask | low_bits_mask);
            mask_bit = ~((0xffffffffffffffff << (inter_itr_f)));
            while (inter_itr_f != 64) {
                int a_plus = __builtin_popcountll(a.idx2 & mask_bit);
                int b_plus = __builtin_popcountll(b.idx2 & mask_bit);
                
                a_vec_itr += a_plus;
                b_vec_itr += b_plus;
                res += __builtin_popcountll(*a_vec_itr & *b_vec_itr);
                
                inter_itr_b = inter_itr_f;
                inter_itr_f = intersection._Find_next(inter_itr_b);
                mask_bit = ~((0xffffffffffffffff << (inter_itr_f)) | ((0xffffffffffffffff >> (63 - inter_itr_b)) >> 1));
//                high_bits_mask = (0xffffffff << (inter_itr_f));
//                low_bits_mask = (0xffffffff >> (32 - inter_itr_b));
//                mask_bit ^= (high_bits_mask | low_bits_mask);
//                mask_bit = ~(high_bits_mask | low_bits_mask);
            }
            return res;
        }
        
        inline int Deal_2_pressed_o_32(const two_pressed_pack &a, const two_pressed_pack &b) {
            int res = 0;
            std::bitset<32> intersection = a.idx2 & b.idx2;
            if (!intersection.any()) {
                return 0;
            }
            
            auto a_vec_itr = (const_cast<two_pressed_pack &>(a).val.begin());
            auto b_vec_itr = const_cast<two_pressed_pack &>(b).val.begin();
            
            uint32_t inter_itr_b = 0;
            uint32_t inter_itr_f = intersection._Find_first();
            uint32_t mask_bit;
//            uint32_t high_bits_mask = (0xffffffff << (inter_itr_f));
//            uint32_t low_bits_mask = 0;
//            mask_bit = ~(high_bits_mask | low_bits_mask);
            mask_bit = ~((0xffffffff << (inter_itr_f)));
            while (inter_itr_f != 32) {
                int a_plus = __builtin_popcountll(a.idx2 & mask_bit);
                int b_plus = __builtin_popcountll(b.idx2 & mask_bit);
                
                a_vec_itr += a_plus;
                b_vec_itr += b_plus;
                res += __builtin_popcountll(*a_vec_itr & *b_vec_itr);
                
                inter_itr_b = inter_itr_f;
                inter_itr_f = intersection._Find_next(inter_itr_b);
                mask_bit = ~((0xffffffff << (inter_itr_f)) | ((0xffffffff >> (31 - inter_itr_b)) >> 1));
//                high_bits_mask = (0xffffffff << (inter_itr_f));
//                low_bits_mask = (0xffffffff >> (32 - inter_itr_b));
//                mask_bit ^= (high_bits_mask | low_bits_mask);
//                mask_bit = ~(high_bits_mask | low_bits_mask);
            }
            return res;
        }

        
        inline CDIST full_dist(const General_data<0, text_num> &a_d,const  General_data<0, text_num> &b_d
                ,const   Structure_data *a_s,const   Structure_data *b_s) const {
            CDIST res = -99999;
            res = JaccardDist<text_num>(a_d.text_bin, b_d.text_bin) + neighDist(a_s->reordered_id, a_s, b_s->reordered_id, b_s);
            if (res < 0){
                throw std::runtime_error("Distance is not valid!!!!!");
            }
            return res;
        }

        inline CDIST full_dist_retrieve(const General_data<0, text_num> &a_d,const  General_data<0, text_num> &b_d
                ,const   Structure_data *a_s,const   Structure_data *b_s) const {
            CDIST res = -99999;
            res = JaccardDist<text_num>(a_d.text_bin, b_d.text_bin) + neighDist_search(a_s->reordered_id, a_s, b_s->reordered_id, b_s);
            if (res < 0){
                throw std::runtime_error("Distance is not valid!!!!!");
            }
            return res;
        }
        
        inline CCONTINUOUS dist_Find_Enter_pt(const std::vector<CCONTINUOUS>& vec1,const  General_data<0, text_num> data1) const{
            CCONTINUOUS res =0;
            for (auto i = 0; i < text_num; ++i){
                res += std::abs(vec1[i]-data1.text_bin[i]);
            }
            return res;
        }
        
        inline CCONTINUOUS full_dist_noNeigh(const General_data<0, text_num> &a_d, const General_data<0, text_num> &b_d){
            CCONTINUOUS res = -99999;
            res = JaccardDist1<text_num>( a_d.text_bin, b_d.text_bin) ;
            if (res < 0){
                throw std::runtime_error("Distance is not valid!!!!!");
            }
            return res;
        }
        template<int num>
        inline CDIST JaccardDist1(const std::bitset<num> &binA,const  std::bitset<num> &binB) const {
            if (binA.count() == 0 || binB.count() == 0) {
                return 1.0;
            }
            return (1 - static_cast<float >((binA & binB).count()) / static_cast<float >((binA | binB).count()));
        }
        };
        
    
    
} // namespace Vamana

#endif //CS_ON_ANNS_METRIC_SPACE_H
