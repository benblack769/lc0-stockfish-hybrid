'''
Script to run games between two engines. Handles crashing, bad output smoothly,
by printing a report of the crash, and then restarting both engines.

Requires 2 command line arguments, the paths to json files describing the engine:

Example json file is:

{
    "engine_path": "/home/benblack/fun_projs/lc0/build/release/lc0",
    "options": [
        "setoption name Hash value 6500",
        "setoption name Threads value 8"
    ],
    "name": "lc0_sf_combined"
}

'''

import sys
import time
import subprocess
import chess
import chess.pgn
import os
import multiprocessing
import json

starttime = 5*60*1000
inctime = 5*1000

def get_bestmove(file,outfile):
    num_lines_split = 0
    while True:
        line = file.readline()
        #print(line)
        outfile.write(line)
        if not line:
            raise RuntimeError("werid output from engine")

        if "bestmove" in line:
            bestmove_start = line.index("bestmove")
            line = line[bestmove_start:]
            outfile.flush()
            return line.split()[1]

def construct_pos_str(moves):
    res = "position startpos moves "
    for m in moves:
        res += (m + " ")
    return res + "\n"

def time_exec(timed_fn):
    start = time.clock()
    timed_fn()
    end = time.clock()
    duration = end - start
    return int(duration * 1000)

class Timer:
    def __init__(self,sw,iw,sb,ib):
        self.wtime = sw
        self.btime = sb
        self.winc = iw
        self.binc = ib

    def update_time(self,white_turn,time_elapsed):
        if white_turn:
            self.wtime -= time_elapsed
            self.wtime += self.winc
        else:
            self.btime -= time_elapsed
            self.btime += self.binc

    def timeout_win(self):
        print(self.wtime)
        print(self.btime)
        if self.wtime < 0:
            return "0-1"
        elif self.btime < 0:
            return "1-0"
        else:
            return "none"

    def go_cmd(self):
        return ("go " +
            " wtime " + str(self.wtime) +
            " btime " + str(self.btime) +
            " winc " + str(self.winc) +
            " binc " + str(self.binc) + "\n")

def moves_to_board(moves):
    board = chess.Board()
    for m in moves:
        board.push_uci(m)
    return board


def terminal_result(moves):
    board = moves_to_board(moves)
    return board.result(claim_draw=True)


class Engine:
    def __init__(self,engine_info,process,read_pipe,write_pipe,game_idx):
        self.name = engine_info['name']
        self.info = engine_info
        self.exec_path = engine_info['engine_path']
        self.process = process
        self.read_pipe = read_pipe
        self.write_pipe = write_pipe
        for option in engine_info['options']:
            self.write_pipe.write(option+"\n")
        stdoutfname = "games/"+str(game_idx)+self.name + ".txt"
        self.stdoutfile = open(stdoutfname,'a')

    def make_move(self,movelist,timer):
        print("engine ",self.name)
        position_str = construct_pos_str(movelist)
        timer_str = timer.go_cmd()
        print(position_str)
        print(timer_str)
        self.write_pipe.write(position_str)
        self.write_pipe.write(timer_str)
        self.write_pipe.flush()

        bestmove = get_bestmove(self.read_pipe,self.stdoutfile)
        return bestmove

    def close(self):
        self.process.terminate()

        self.stdoutfile.close()



def board_to_pgn(e1,e2,board,result,write_file,write_idx):
    hdrs = {
        "Event": "test_chess_comp",
        "White": e1,
        "Black": e2,
        "Result": result,
    }
    #game = chess.pgn.Game(headers=hdrs)
    #board = moves_to_board(moves)
    #game.setup(board)
    #with open(write_file,'w') as wfile:
    print("game started!!!!!\n\n\n")
    game = chess.pgn.Game.from_board(board)
    for k,v in hdrs.items():
        game.headers[k] = v

    print(game,file=write_file,flush=True)
    write_filename = "games/{}".format(write_idx)
    #subprocess.check_call("aws s3 cp {} s3://script-wars-deploy/chess_games/{}".format(write_filename,write_idx),shell=True)
    #write_file.write(repr(game))
    #return

def create_engines(e1info,e2info,game_idx):
    PIPE = subprocess.PIPE
    eng1_proc = subprocess.Popen("exec {}".format(e1info['engine_path']),stdin=PIPE,stdout=PIPE,shell=True,text=True)
    eng2_proc = subprocess.Popen("exec {}".format(e2info['engine_path']),stdin=PIPE,stdout=PIPE,shell=True,text=True)
    #eng2_proc = subprocess.Popen([e2name],stdin=open(write2_fifo),stdout=open(read2_fifo,'w'))
    write1 = eng1_proc.stdin #open(write1_fifo,'w')
    write2 = eng2_proc.stdin #open(write2_fifo,'w')
    read1 = eng1_proc.stdout #open(read1_fifo,'r')
    read2 = eng2_proc.stdout #open(read2_fifo,'r')

    engine1 = Engine(e1info,eng1_proc,read1,write1,game_idx)
    engine2 = Engine(e2info,eng2_proc,read2,write2,game_idx)

    return engine1,engine2


def process_game(eng1,eng2,game_idx):

    cur_eng = eng1
    prev_eng = eng2

    sw = starttime
    sb = starttime
    iw = inctime
    ib = inctime

    board = chess.Board()
    timer = Timer(sw,iw,sb,ib)
    moves = []
    white_turn = True

    while not board.is_game_over(claim_draw=True):
        while True:
            try:
                start = time.time()
                move = cur_eng.make_move(moves,timer)
                end = time.time()
                duration = int((end - start) * 1000)
                break
            except (subprocess.CalledProcessError,RuntimeError,ValueError) as err:
                cur_eng.close()
                prev_eng.close()
                cur_eng,prev_eng = create_engines(cur_eng.info,prev_eng.info,game_idx)
                with open("games/{}crash_log".format(game_idx),'a') as crash_log:
                    crash_log.write("ERROR in {}:\n{}\n\n".format(cur_eng.name,str(err)))

        timer.update_time(white_turn,duration)

        timeout_result = timer.timeout_win()
        if timeout_result != "none":
            return (board,timeout_result)

        board.push_uci(move)
        moves.append(move)

        temp_eng = cur_eng
        cur_eng = prev_eng
        prev_eng = temp_eng

        white_turn = not white_turn


    return (board,board.result(claim_draw=True))

def run_game(e1info,e2info,outfilename,idx):
    eng1,eng2 = create_engines(e1info,e2info,idx)

    board,result = process_game(eng1,eng2,idx)

    with open(outfilename,'w') as outfile:
        board_to_pgn(e1info['name'],e2info['name'],board,result,outfile,idx)

    eng1.close()
    eng2.close()

def run_many(engine1_info,engine2_info,times):

    os.mkdir("games")

    white = engine1_info
    black = engine2_info
    for game_idx in range(times):
        run_game(white, black, "games/{}".format(game_idx),game_idx)

        t = black
        black = white
        white = t


def main():
    assert len(sys.argv) == 3, "needs 2 command line arguments, the names of the two engines"

    eng1_name = sys.argv[1]
    eng2_name = sys.argv[2]

    eng1_info = json.load(open(eng1_name))
    eng2_info = json.load(open(eng2_name))

    run_many(eng1_info,eng2_info,100)

if __name__ == "__main__":
    main()
