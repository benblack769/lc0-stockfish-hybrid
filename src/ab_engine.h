#pragma once
namespace ab_engine{

void start_engine();
void run_command(std::string cmd);
void run_go_command();
void wait_until_bestmove();
std::string get_command_line();

}
