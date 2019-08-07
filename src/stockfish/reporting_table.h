#pragma once
#include <vector>
#include <chrono>
#include "types.h"
#include "utils/optional.h"
#include "utils/exception.h"

struct CompareablePosition {
    Bitboard byTypeBB[PIECE_TYPE_NB];
    Bitboard byColorBB[COLOR_NB];
    bool active_color; //white = 0, black = 1
    int castlingRights;
    int enpassant;
    bool operator == (const CompareablePosition & other) const;
    bool operator != (const CompareablePosition & other) const{ return !((*this) == other); }
    uint64_t Hash() const;
    bool ep_diff(const CompareablePosition & o)const{return o.enpassant != enpassant;}
    //std::string fen()const;
};
struct CompareableMove{
    static constexpr int number_add = 49;
    static constexpr int letter_add = 97;
    static constexpr int mask8 = 8-1;
    static constexpr int bits8 = 3;
    int16_t move=-1;
    CompareableMove() = default;
    CompareableMove(std::string s);
    bool is_set()const{
        return move != -1;
    }
    std::string to_string()const;
    bool operator == (CompareableMove other) const {
        return move == other.move;
    }
};
using CompareableMoveList = std::vector<CompareableMove>;
namespace std {
  template <> struct hash<CompareablePosition>
  {
    size_t operator()(const CompareablePosition & x) const
    {
        return x.Hash();
      /* your code here, e.g. "return hash<int>()(x.value);" */
    }
  };
}
struct ABTableEntry{
    CompareableMoveList moves;
    int search_depth;
};
struct GlobalCollectionInfo{
    int guided_choices=0;
    int not_guided_choices=0;
    double avg_depth=10;
    bool found_mate = false;
};

struct TimeHeapReturn{
    CompareablePosition pos;
    CompareableMoveList moves_to_pos;
    int search_depth;
};

struct SmallHistogram{
    static constexpr size_t size = 256;
    int depth_reduction_histogram[size] = {0};

    void zero_depth_reduction_hist(){
        for(size_t i = 0; i < size; i++){
            depth_reduction_histogram[i] = 0;
        }
    }
};
std::ostream & operator << (std::ostream & os,const SmallHistogram & sh);
namespace reporting{
struct Parameters{
    int stockfish_mover_tolerance;
    int stockfish_opponent_tolerance;
    int min_ab_depth_valid;
};
void clear();
void remove_old_nodes();
//bool should_set_entry(Key key, int depth,int search_depth,Value val);
void set_ab_entry(CompareablePosition position,CompareableMoveList moves,int search_depth, int64_t microseconds_spent);
void set_mcts_entry(CompareablePosition position, CompareableMoveList moves_to_pos, int nodes_searched);
void set_bestmove_if_exists(CompareablePosition position, CompareableMove bestmove, int bestmove_depth);
TimeHeapReturn pop_calc_position();
lczero::optional<ABTableEntry> get_ab_entry(CompareablePosition position);
void debug();
void set_parameters(Parameters params);
Parameters get_parameters();
GlobalCollectionInfo get_info();
SmallHistogram move_histogram();
void set_found_mate();
inline bool has_found_mate(){
    return get_info().found_mate;
}
//void print_results();
//void save_to_file(std::string fname);
//std::vector<std::pair<CompareablePosition,int>> pos_vals();
void set_bestvalue(Value bestval,int depth);
void set_path_chosen(bool guided);
Value get_bestvalue();
int get_bestvalue_depth();

template<class FnTy>
inline int64_t time_microseconds(FnTy fn){
    auto start = std::chrono::high_resolution_clock::now();
    fn();
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    int64_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    return microseconds;
}
}

inline bool contains(const CompareableMoveList & mvs, CompareableMove mv){
    for(CompareableMove m : mvs){
        if(m == mv){
            return true;
        }
    }
    return false;
}
