
import chess
import chess.pgn
import sys

def main():
    assert len(sys.argv) == 3, "needs 2 command line arguments, the names of the two engines"

    eng1_name = sys.argv[1]
    eng2_name = sys.argv[2]

def board_to_pgn(e1,e2,board,result):
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
    game = chess.pgn.Game.from_board(board)
    for k,v in hdrs.items():
        game.headers[k] = v

    print(game,flush=True)

def process_game(moves):
    board = chess.Board()
    white_turn = True

    for move in moves:
        board.push_uci(move)

    result = board.result(claim_draw=True)
    board_to_pgn("p1","p2",board,result)

if __name__ == "__main__":
    moves = []
    for move in sys.argv[1:]:
        moves.append(move)
    process_game(moves)
