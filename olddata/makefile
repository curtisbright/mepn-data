primechecker: primechecker.c ../pprime_p.c
	gcc primechecker.c -O3 -o primechecker -std=c99 -lgmp -I /home/cbright/gmp-5.1.0
primechecker2: primechecker2.c
	gcc primechecker2.c -O3 -o primechecker2 -std=c99 -lgmp -I /home/cbright/gmp-5.1.0
usolved: usolved.c
	gcc usolved.c -O3 -o usolved -std=c99 -lgmp -I /home/cbright/gmp-5.1.0
debug: primechecker.c ../pprime_p.c
	gcc primechecker.c -o primechecker -std=c99 -lgmp -I /home/cbright/gmp-5.1.0 -g
prune: prune.c
	gcc prune.c -O3 -o prune -std=c99
data: data.c
	gcc data.c -O3 -o data -std=c99
output: output.c
	gcc output.c -O3 -o output -std=c99 -lgmp
