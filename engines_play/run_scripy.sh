mkdir official_games

python main.py official_games/lc0_sf_hybrid-sf-0.5-1 lc0_sf_hybrid.cfg stockfish.cfg 30000 1000 1000
python main.py official_games/lc0-sf-0.5-1 lc0_orig.cfg stockfish.cfg 30000 1000 1000
python main.py official_games/lc0_sf_hybrid-lc0-0.5-1 lc0_sf_hybrid.cfg lc0_orig.cfg 30000 1000 1000

python main.py official_games/lc0_sf_hybrid-sf-1-1 lc0_sf_hybrid.cfg stockfish.cfg 60000 1000 1500
python main.py official_games/lc0-sf-1-1 lc0_orig.cfg stockfish.cfg 60000 1000 1500
python main.py official_games/lc0_sf_hybrid-lc0-1-1 lc0_sf_hybrid.cfg lc0_orig.cfg 60000 1000 1500

python main.py official_games/lc0_sf_hybrid-sf-3-2 lc0_sf_hybrid.cfg stockfish.cfg 180000 2000 3000
python main.py official_games/lc0-sf-3-2 lc0_orig.cfg stockfish.cfg 180000 2000 3000
python main.py official_games/lc0_sf_hybrid-lc0-3-2 lc0_sf_hybrid.cfg lc0_orig.cfg 180000 2000 3000

python main.py official_games/lc0_sf_hybrid-sf-5-5 lc0_sf_hybrid.cfg stockfish.cfg 300000 5000 5000
python main.py official_games/lc0-sf-5-5 lc0_orig.cfg stockfish.cfg 300000 5000 5000
python main.py official_games/lc0_sf_hybrid-lc0-5-5 lc0_sf_hybrid.cfg lc0_orig.cfg 300000 5000 5000

python main.py official_games/lc0_sf_hybrid-sf-10-0 lc0_sf_hybrid.cfg stockfish.cfg 600000 0 5000
python main.py official_games/lc0-sf-10-0 lc0_orig.cfg stockfish.cfg 600000 0 5000
python main.py official_games/lc0_sf_hybrid-lc0-10-0 lc0_sf_hybrid.cfg lc0_orig.cfg 600000 0 5000

python main.py official_games/lc0_sf_hybrid-sf-15-10 lc0_sf_hybrid.cfg stockfish.cfg 900000 10000 10000
python main.py official_games/lc0_sf_hybrid-lc0-15-10 lc0_sf_hybrid.cfg lc0_orig.cfg 900000 10000 10000
python main.py official_games/lc0-sf-15-10 lc0_orig.cfg stockfish.cfg 900000 10000 10000

#python main.py testeng lc0_sf_hybrid.cfg stockfish.cfg 30000 1000
