#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <algorithm>
#include <vector>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ext/stdio_filebuf.h>
#include "ab_engine.h"

using namespace std;

#define r_assert(val) { \
    if(!(val)){ \
        std::cout << ("ERROR: " #val ) << std::endl; \
        exit(1); \
    } \
}

ofstream write_file;
bool pipe_created = false;

const char * write_pipename = "sf_write.pipe";

void spawn_sf_main_thread();

namespace ab_engine{

void start_engine(){
    r_assert(!pipe_created);
    remove(write_pipename);
    r_assert(mkfifo(write_pipename,0777) == 0);

    spawn_sf_main_thread();
    write_file.open(write_pipename);
    pipe_created = true;
}
void run_command(std::string cmd){
    r_assert(pipe_created);
    if(cmd.back() != '\n'){
        cmd.push_back('\n');
    }
    write_file << cmd;
    write_file.flush();
}
void run_go_command(){
    run_command("go infinite");
}
}
