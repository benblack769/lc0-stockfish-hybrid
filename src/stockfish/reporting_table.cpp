#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <queue>
#include <utility>
#include "reporting_table.h"
#include <stdlib.h>
#include <thread>
#include "misc.h"
#include "chess/board.h"
#include "utils/hashcat.h"
#include "bitboard.h"

using namespace std;

#define r_assert(val) { \
    if(!(val)){ \
        write_out << #val << std::endl; \
        exit(1); \
    } \
}


mutex global_lock;
mutex params_lock;


mutex bestval_mutex;
Value curbestval;
CompareableMoveList current_pv;
reporting::Parameters global_params;
GlobalCollectionInfo cur_glob_info;
int curdepth = -1;

static constexpr int MIN_SEARCH_DEPTH = 2;
static constexpr int MIN_SEARCH_NODES = 3;
constexpr size_t INVALID_LOC = 0xff0000000000;

struct TableEntry{
    ComparableMoveValList moves;
    CompareableMoveList moves_to_pos;
    CompareableMove bestmove;
    int bestmove_depth=-1;
    int64_t ab_time=0;
    size_t item_location=INVALID_LOC;//pointer to heap location
    int nodes_searched=0;
    int calced_search_depth=MIN_SEARCH_DEPTH-1;
    bool is_calulating=false;
};
using heap_iter = typename unordered_map<CompareablePosition,TableEntry>::iterator;
struct TimeRatioItem{
    std::pair<const CompareablePosition,TableEntry> * hash_ptr;
    int64_t ab_time;
    int mcts_nodes;
    bool operator < (const TimeRatioItem & o)const{
        return ab_time && o.ab_time ?
            (ab_time * o.mcts_nodes) > (mcts_nodes * o.ab_time) :
            ab_time > o.ab_time;
    }
};

void tr_swap(TimeRatioItem & i1, TimeRatioItem & i2){
    swap(i1.hash_ptr->second.item_location,i2.hash_ptr->second.item_location);
    swap(i1,i2);
}
#define r_assert2(cond) {if(!(cond)) {std::cout << ("condition: " #cond " failed on line ") <<  __LINE__ << std::endl;}}
class TimeRatioHeap{
    unordered_map<CompareablePosition,TableEntry> item_location;
    vector<TimeRatioItem> heap;
public:
    lczero::optional<ABTableEntry> moves(CompareablePosition pos){
        heap_iter iter = item_location.find(pos);
        if(iter == item_location.end()){
            return lczero::optional<ABTableEntry>();
        }
        else{
            TableEntry & entry = iter->second;
            return lczero::optional<ABTableEntry>(ABTableEntry{entry.moves,iter->second.calced_search_depth});
        }
    }
    //saves some search info, but clears it for next move
    /*void new_search(){
        heap.clear();
        item_location.clear();
    }*/
    void finished_ab_calc(CompareablePosition pos, ComparableMoveValList moves, int search_depth, int64_t ab_time_spent){
        heap_iter h_iter = item_location.find(pos);
        if(h_iter == item_location.end()){
            throw runtime_error("got position without location");
        }
        //if(pos.ep_diff(h_iter->first)){
        //    return;//don't change value of node if en passant differnet
        //}
        TableEntry & entry = h_iter->second;
        entry.moves = moves;
        entry.ab_time += ab_time_spent;
        if(entry.calced_search_depth != search_depth){
            throw runtime_error("didn't increment search_depth properly");
        }
        entry.calced_search_depth = search_depth;
        if(!entry.is_calulating){
            throw runtime_error("finished without starting");
        }
        entry.is_calulating = false;

        /*if(entry.item_location != INVALID_LOC){
            TimeRatioItem & item = heap.at(entry.item_location);
            item.ab_time = entry.ab_time;
            item.mcts_nodes = entry.nodes_searched;
            rearange_item(entry.item_location);
        }
        else{*/
        TimeRatioItem item = {&(*h_iter),entry.ab_time,entry.nodes_searched};
        push_heap(item);
        //}
    }
    void update_mcts(CompareablePosition pos, CompareableMoveList moves_to_pos, int mcts_nodes){
        TableEntry & entry = item_location[pos];
        auto h_iter = item_location.find(pos);
        //if(pos.ep_diff(h_iter->first)){
        //    return;//don't change value of node if en passant differnet
        //}
        entry.moves_to_pos = moves_to_pos;
        entry.nodes_searched += mcts_nodes;
        if(entry.is_calulating){
            return;
        }
        if(entry.item_location == INVALID_LOC){
            push_heap(TimeRatioItem{&(*h_iter),entry.ab_time,entry.nodes_searched});
        }
        else{
            TimeRatioItem & item = heap.at(entry.item_location);
            item.ab_time = entry.ab_time;
            item.mcts_nodes = entry.nodes_searched;
            rearange_item(entry.item_location);
        }
    }
    void clear(){
        item_location.clear();
        heap.clear();
    }
    bool empty(){
        return heap.size() == 0;
    }
    TimeHeapReturn pop(){
        if(!heap.size()){
            throw runtime_error("popped when empty");
        }
        const CompareablePosition & pos = heap.front().hash_ptr->first;
        TableEntry & entry = heap.front().hash_ptr->second;
        r_assert2(entry.item_location == 0);
        entry.is_calulating = true;
        entry.calced_search_depth++;
        pop_heap();
        entry.item_location = INVALID_LOC;//invalid location
        return TimeHeapReturn {pos,entry.moves_to_pos,entry.calced_search_depth};
    }
    SmallHistogram move_histogram(){
        SmallHistogram hist;
        for(const auto & pos_entry : item_location){
            size_t entry = pos_entry.second.moves.size();
            size_t add = pos_entry.second.nodes_searched;
            hist.depth_reduction_histogram[entry] += add;
        }
        return hist;
    }
private:
    bool rearrange_up(size_t loc){
        TimeRatioItem & cur_item = heap.at(loc);
        if(loc > 0){
            size_t prev_loc = (loc-1)/2;
            TimeRatioItem & prev_item = heap.at(prev_loc);
            if(prev_item < cur_item){
                tr_swap(cur_item,prev_item);
                rearrange_up(prev_loc);
                return true;
            }
        }
        return false;
    }
    void raarange_down(size_t loc){
        TimeRatioItem & cur_item = heap.at(loc);
        size_t new_loc1 = loc*2+1;
        size_t new_loc2 = loc*2+2;
        if(new_loc1 >= heap.size()){
            return;
        }
        TimeRatioItem & next_item1 = heap.at(new_loc1);
        if(cur_item < next_item1){
            if(new_loc2 >= heap.size() || heap[new_loc2] < next_item1){
                tr_swap(cur_item,next_item1);
                raarange_down(new_loc1);
            }
            else{
                TimeRatioItem & next_item2 = heap.at(new_loc2);
                tr_swap(cur_item,next_item2);
                raarange_down(new_loc2);
            }
        }
    }
    void rearange_item(size_t loc){
        if(!rearrange_up(loc)){
            raarange_down(loc);
        }
    }
    void push_heap(TimeRatioItem item){
        int loc = heap.size();
        item.hash_ptr->second.item_location = loc;
        heap.push_back(item);
        rearrange_up(loc);
        //std::push_heap(heap.begin(),heap.end());
    }
    void pop_heap(){
        tr_swap(heap.front(),heap.back());
        heap.pop_back();
        if(heap.size()){
            raarange_down(0);
        }
    }
};
TimeRatioHeap ratio_heap;

bool CompareablePosition::operator == (const CompareablePosition & other) const {
    return std::equal(byTypeBB,byTypeBB+PIECE_TYPE_NB,other.byTypeBB) &&
           std::equal(byColorBB,byColorBB+COLOR_NB,other.byColorBB) &&
           active_color == other.active_color &&
           castlingRights == other.castlingRights &&
           enpassant == other.enpassant;
}
uint64_t CompareablePosition::Hash() const {
    using namespace lczero;
    uint64_t hash = 0;
    for(int i = 0; i < PIECE_TYPE_NB; i++){
        hash = HashCat(hash,byTypeBB[i]);
    }
    for(int i = 0; i < COLOR_NB; i++){
        hash = HashCat(hash,byColorBB[i]);
    }

    hash = HashCat(hash,active_color);
    hash = HashCat(hash,castlingRights);
    hash = HashCat(hash,enpassant);

    return hash;
}

CompareableMove::CompareableMove(std::string s){
    int srcfile = (s[0] - letter_add) << (bits8*0);
    int srcrank = (s[1] - number_add) << (bits8*1);
    int destfile = (s[2] - letter_add) << (bits8*2);
    int destrank = (s[3] - number_add) << (bits8*3);
    move = srcfile  | srcrank |
           destfile | destrank;
    if(s.size() > 4){
        int promoteval = -1;
        switch(s[4]){
            case 'b': promoteval = 1; break;
            case 'n': promoteval = 2; break;
            case 'q': promoteval = 3; break;
            case 'r': promoteval = 4; break;
        }
        move |= promoteval << (bits8*4);
    }
}
std::string CompareableMove::to_string()const{
    if(!is_set()){
        return "none";
    }
    char srcfile =  ((move >> (bits8*0)) & mask8) + letter_add;
    char srcrank =  ((move >> (bits8*1)) & mask8) + number_add;
    char destfile = ((move >> (bits8*2)) & mask8) + letter_add;
    char destrank = ((move >> (bits8*3)) & mask8) + number_add;
    int promotion = (move >> (bits8*4));
    std::string result = {srcfile,srcrank,destfile,destrank};
    if(promotion){
        const char add_letter[] = "_bnqr";
        result.push_back(add_letter[promotion]);
    }
    return result;
}
std::ostream & operator << (std::ostream & os,const SmallHistogram & sh){
    os << "{";
    for(size_t i = 0; i < sh.size; i++){
        if(sh.depth_reduction_histogram[i]){
            os << i << ":" << sh.depth_reduction_histogram[i] << ",";
        }
    }
    os << "}";
    return os;
}

namespace reporting{
void clear(){
    global_lock.lock();
    std::cout << "\n\n\ncleared!!!\n\n\n" << std::endl;
    ratio_heap.clear();
    current_pv.clear();
    curdepth = -1;
    cur_glob_info = GlobalCollectionInfo();
    global_lock.unlock();
}
GlobalCollectionInfo get_info(){
    global_lock.lock();
    GlobalCollectionInfo res = cur_glob_info;
    global_lock.unlock();
    return res;
}
void set_parameters(Parameters params){
    params_lock.lock();
    global_params = params;
    params_lock.unlock();
}
Parameters get_parameters(){
    params_lock.lock();
    Parameters res = global_params;
    params_lock.unlock();
    return res;
}
TimeHeapReturn pop_calc_position(){
    const std::chrono::milliseconds sleep_time(10);
    while(true){
        global_lock.lock();
        if(!ratio_heap.empty()){
            break;
        }
        global_lock.unlock();
        //cout << "slept\n";
        std::this_thread::sleep_for(sleep_time);
    }
    TimeHeapReturn res = ratio_heap.pop();
    global_lock.unlock();
    return res;
}
void set_ab_entry(CompareablePosition position, ComparableMoveValList moves, int search_depth, int64_t microseconds_spent){
    global_lock.lock();

    ratio_heap.finished_ab_calc(position,moves,search_depth,microseconds_spent);

    global_lock.unlock();
}
void set_found_mate(){
    global_lock.lock();

    cur_glob_info.found_mate = true;

    global_lock.unlock();
}
void set_mcts_entry(CompareablePosition position, CompareableMoveList moves_to_pos, int nodes_searched){
    global_lock.lock();

    ratio_heap.update_mcts(position,moves_to_pos,nodes_searched);

    global_lock.unlock();
}
using ab_return_type = lczero::optional<ABTableEntry>;
ab_return_type get_ab_entry(CompareablePosition position){
    global_lock.lock();

    ab_return_type res = ratio_heap.moves(position);

    global_lock.unlock();
    return res;
}

void debug(){
    global_lock.lock();
    //cout << "ab size: " << ab_table.size() << "\n";
    //cout << "mcts size: " << mcts_table.size() << "\n";

    global_lock.unlock();
}
void set_path_chosen(bool guided){
    global_lock.lock();
    if(guided){
        cur_glob_info.guided_choices++;
    }
    else{
        cur_glob_info.not_guided_choices++;
    }
    global_lock.unlock();
}

SmallHistogram move_histogram(){
    global_lock.lock();
    SmallHistogram hist = ratio_heap.move_histogram();
    global_lock.unlock();
    return hist;
}
void set_bestvalue(Value bestval,int depth){
    bestval_mutex.lock();
    if(depth > curdepth){
        curbestval = bestval;
        curdepth = depth;
    }
    bestval_mutex.unlock();
}
Value get_bestvalue(){
    bestval_mutex.lock();
    Value val = curbestval;
    bestval_mutex.unlock();
    return val;
}
int get_bestvalue_depth(){
    bestval_mutex.lock();
    int d = curdepth;
    bestval_mutex.unlock();
    return d;
}

}
