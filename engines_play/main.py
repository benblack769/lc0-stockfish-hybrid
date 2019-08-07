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
import subprocess
import os

def run_game(e1info,e2info,idx):
    args = [
        "python",
        "run_one.py",
        str(idx),
        e1info,
        e2info,
    ]
    subprocess.check_call(args)


def run_many(engine1_name,engine2_name,times):

    os.mkdir("games")

    white = engine1_name
    black = engine2_name
    for game_idx in range(times):
        run_game(white, black, game_idx)

        t = black
        black = white
        white = t


def main():
    assert len(sys.argv) == 3, "needs 2 command line arguments, the names of the two engines"

    eng1_name = sys.argv[1]
    eng2_name = sys.argv[2]

    eng1_name = eng1_name
    eng2_name = eng2_name

    run_many(eng1_name,eng2_name,100)

if __name__ == "__main__":
    main()
