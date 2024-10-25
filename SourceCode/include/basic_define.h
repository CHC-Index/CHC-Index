#ifndef CS_ON_ANNS_BASIC_DEFINE_H
#define CS_ON_ANNS_BASIC_DEFINE_H

#include <cstdint>
#include <vector>
#include <string>
#include <map>

#define CLOCK std::chrono::high_resolution_clock::now()
#define TIME_MS(s,e) std::chrono::duration_cast<std::chrono::milliseconds>(e - s).count()
#define TIME_US(s,e) std::chrono::duration_cast<std::chrono::microseconds>(e - s).count()
#define UACOUT std::cout.setstate(std::ios::failbit);
#define EACOUT std::cout.clear();
namespace chci{
    using CLAYER = int32_t;
    using CDEGREE = uint16_t;
    using CNODEID = int32_t;
    using CEDGEID = int32_t;
    using CCORE = int32_t;
    using CUINT = uint32_t;
    using CCHAR = char;
    using adj_t =  std::vector<std::vector<CNODEID>>;
    using CDIST = float;
    using CCONTINUOUS = float;
    using CSTATUS = int32_t;

    enum Dataset_Type{small_dblp,DBLP, IMDB_Person, IMDB_Movie, FourSquare, CORA, Twitch, LiveJournal, Facebook,
                        DBpedia_Basketballplayer, DBpedia_Automobile, DBpedia_Soccerplayer, Github,
                        FB0, FB107, FB348, FB414, FB686, FB698, FB1684, FB1912, FB3437, FB3980,
                        FB_MERGE, FB_MERGE_ORI};
    enum SearchNodeIdType{origin, cnns};
    enum StructureMethod{baseline, opt2, B};
    enum Cmodel{core, truss};
    enum __Metric{HYB, TEXT};
}
#endif
