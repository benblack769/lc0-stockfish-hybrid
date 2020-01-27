'''
Script to run games between two engines. Handles crashing, bad output smoothly,
by printing a report of the crash, and then restarting both engines.

Requires 2 command line arguments, the paths to json files describing the engine:

Example json file is:

{
    "engine_path": "/home/ben/fun_projs/lc0/build/release/lc0",
    "options": [
        "setoption name Hash value 6500",
        "setoption name Threads value 8"
    ],
    "name": "lc0_sf_combined"
}

'''

import sys
import subprocess
import os

def run_game(e1info,e2info,folder,idx,timeout):
    args = [
        "python",
        os.path.abspath("run_one.py"),
        str(idx),
        os.path.abspath(e1info),
        os.path.abspath(e2info),
        str(starttime),
        str(inctime)
    ]
    print(" ".join(args))
    while True:
        try:
            subprocess.check_call(args,cwd=folder,timeout=timeout)#,shell=True)
            return
        except subprocess.TimeoutExpired:
            with open(os.path.join(folder,"{}crash_log".format(idx)),'a') as crash_log:
                crash_log.write("game timed out! restarting\n")



def run_many(engine1_name,engine2_name,folder,times,timeout):

    os.mkdir(folder)

    white = engine1_name
    black = engine2_name
    for game_idx in range(times):
        run_game(white, black, folder, game_idx,timeout)

        t = black
        black = white
        white = t


if __name__ == "__main__":
    assert len(sys.argv) == 7, "needs 6 command line arguments, the output folder and the names of the two engines, starttime, inctime, timeoutmax "

    outfold_name = sys.argv[1]
    eng1_name = sys.argv[2]
    eng2_name = sys.argv[3]
    starttime = sys.argv[4]
    inctime = sys.argv[5]
    timeout_max = float(sys.argv[6])

    eng1_name = eng1_name
    eng2_name = eng2_name

    run_many(eng1_name,eng2_name,outfold_name,50,timeout_max)
