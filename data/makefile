primechecker: primechecker.c ../pprime_p.c
	gcc primechecker.c -O3 -o primechecker -std=c99 -lgmp -I /home/cbright/gmp-5.0.4
debug: primechecker.c ../pprime_p.c
	gcc primechecker.c -o primechecker -std=c99 -lgmp -I /home/cbright/gmp-5.0.4 -g
prune: prune.c
	gcc prune.c -O3 -o prune -std=c99
data: data.c
	gcc data.c -O3 -o data -std=c99
