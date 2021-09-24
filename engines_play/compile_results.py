import sys

import chess
import chess.pgn
from collections import Counter
import os

def get_results(folder):
    results = []
    for idx in range(500):
        pgn_path = os.path.join(folder,str(idx))
        if not os.path.exists(pgn_path):
            continue
        with open(pgn_path) as pgn_file:
            first_game = chess.pgn.read_game(pgn_file)
            res = first_game.headers["Result"]
            if res == "1-0":
                results.append(first_game.headers["White"])
            elif res == "0-1":
                results.append(first_game.headers["Black"])
            elif res == "1/2-1/2":
                results.append("drawn")
            else:
                pass
    return Counter(results)

if __name__ == "__main__":
    assert len(sys.argv) == 2

    folder = sys.argv[1]
    print(get_results(folder))
