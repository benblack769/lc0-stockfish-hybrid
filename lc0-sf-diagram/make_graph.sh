cutoff=$1
indir=$2
outdir=$3

python make_diagram.py $cutoff  $indir/sf_opening.json | dot -Tsvg -o $outdir/sf_opening.svg
python make_diagram.py $cutoff  $indir/sf_middle.json | dot -Tsvg -o $outdir/sf_middle.svg
python make_diagram.py $cutoff  $indir/sf_tactical.json | dot -Tsvg -o $outdir/sf_tactical.svg
python make_diagram.py $cutoff  $indir/nosf_opening.json | dot -Tsvg -o $outdir/nosf_opening.svg
python make_diagram.py $cutoff  $indir/nosf_middle.json | dot -Tsvg -o $outdir/nosf_middle.svg
python make_diagram.py $cutoff  $indir/nosf_tactical.json | dot -Tsvg -o $outdir/nosf_tactical.svg
