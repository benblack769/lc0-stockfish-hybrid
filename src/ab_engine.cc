#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include <fstream>
#include <mutex>
#include <thread>
#include "ab_engine.h"
#include "stockfish/reporting_table.h"


#define r_assert(val) { \
    if(!(val)){ \
        std::cout << ("ERROR: " #val ) << std::endl; \
        exit(1); \
    } \
}
std::mutex line_lock;

bool is_started = false;
std::queue<std::string> line_queue;


void spawn_sf_main_thread();

namespace ab_engine{

void start_engine(){
    line_lock.lock();
    r_assert(!is_started);

    spawn_sf_main_thread();
    reporting::clear();

    is_started = true;
    line_lock.unlock();
}
void run_command(std::string cmd){
    line_lock.lock();

    r_assert(is_started);
    if(cmd.back() == '\n'){
        cmd.pop_back();
    }
    line_queue.push(cmd);

    line_lock.unlock();
}
std::string get_command_line(){
    const std::chrono::milliseconds sleep_time(10);
    while(true){
        line_lock.lock();
        if(!line_queue.empty()){
            break;
        }
        line_lock.unlock();
        std::this_thread::sleep_for(sleep_time);
    }
    std::string res = line_queue.front();
    line_queue.pop();

    line_lock.unlock();

    return res;
}
void run_go_command(){
    run_command("go infinite");
}
}
