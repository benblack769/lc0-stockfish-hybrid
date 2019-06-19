/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad
  Copyright (C) 2015-2019 Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <thread>
#include <fstream>

#include "bitboard.h"
#include "position.h"
#include "search.h"
#include "thread.h"
#include "tt.h"
#include "uci.h"
#include "syzygy/tbprobe.h"


namespace PSQT {
  void init();
}

std::ifstream read_file_;
//std::ofstream write_file_;

const char * _read_pipename = "sf_write.pipe";
//const char * _write_pipename = "sf_read.pipe";

std::istream * _sf_read_in;
//std::ostream * _sf_write_out;

void create_stdin_pipe(){
    read_file_.open(_read_pipename);
    //write_file_.open(_write_pipename);
    //_sf_write_out = &write_file_;
    _sf_read_in = &read_file_;
}

void sf_thread_main() {

  create_stdin_pipe();

  write_out << engine_info() << std::endl;

  UCI::init(Options);
  PSQT::init();
  Bitboards::init();
  Position::init();
  Bitbases::init();
  Search::init();
  Pawns::init();
  Threads.set(Options["Threads"]);
  Search::clear(); // After threads are up

  int argc = 1;
  char** argv = NULL;
  UCI::loop(argc, argv);

  Threads.set(0);
}
void spawn_sf_main_thread(){
    std::thread t(sf_thread_main);
    t.detach();
}
