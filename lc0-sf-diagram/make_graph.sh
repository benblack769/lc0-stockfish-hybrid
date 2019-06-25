cutoff=$1
indir=$2
outdir=$3

python make_diagram.py $cutoff  $indir/main.json | dot -Tsvg -o $outdir/main.svg
python make_diagram.py $cutoff  $indir/middle.json | dot -Tsvg -o $outdir/middle.svg
python make_diagram.py $cutoff  $indir/tactical.json | dot -Tsvg -o $outdir/tactical.svg
python make_diagram.py $cutoff  $indir/nosf_main.json | dot -Tsvg -o $outdir/nosf_main.svg
python make_diagram.py $cutoff  $indir/nosf_middle.json | dot -Tsvg -o $outdir/nosf_middle.svg
python make_diagram.py $cutoff  $indir/nosf_tactical.json | dot -Tsvg -o $outdir/nosf_tactical.svg
